// xmlsonator - xml to json converter for node using the libxml++ sax parser API
// Andrew Zdenek - ajz01
// Oct 27, 2015

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>
#include <v8.h>
#include <uv.h>
#include <stdio.h>
#include <memory.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <string>
#include <vector>
#include <deque>

using namespace std;
using namespace v8;
using namespace node;

// used to initialize a libml SAX2 parser and construct a
// javascript Object containing the json equivlant of an
// xml representation passed as a node Buffer.
class Xmlsonator
{
public:

  // constructor. set return value object and v8 isolate.
  Xmlsonator(Local<Object> obj, Isolate* isolate) {
    this->object_ = obj;
    this->isolate_ = isolate;
    this->inelem = false;
    this->startelem = false;
  }

static inline std::string trim(std::string& str)
{
str.erase(0, str.find_first_not_of(" \t\n"));       //prefixing spaces
str.erase(str.find_last_not_of(" \t\n")+1);         //surfixing spaces
return str;
}

  // handle element inner text
  static void characters(void * ctx, const xmlChar * ch, int len) {
    Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
    if(xsr.startelem) {
      Local<Object> obj = xsr.ostack.back();
      string str((char*)ch, len);
      str = trim(str);
      if(!str.empty()) {//if(str.find_first_not_of(" \t\n") != string::npos) {
        //printf("characters: %s has: %i\n", xsr.beginelem, len);
        //Local<Object> tmp = Object::New(xsr.isolate_);
        //tmp->Set(String::NewFromUtf8(xsr.isolate_,xsr.beginelem), String::NewFromUtf8(xsr.isolate_,str.c_str()));
        //obj->Set(String::NewFromUtf8(xsr.isolate_,xsr.beginelem), tmp);
        xsr.buffer += str;
      }
    }
  }

   static void startElementNs( void * ctx,
                               const xmlChar * localname,
                               const xmlChar * prefix,
                               const xmlChar * URI,
                               int nb_namespaces,
                               const xmlChar ** namespaces,
                               int nb_attributes,
                               int nb_defaulted,
                               const xmlChar ** attributes )
   {
      Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      Local<Object> obj = Object::New(xsr.isolate_);
      xsr.ostack.push_back(obj);
      //printf( "startElementNs: name = '%s' prefix = '%s' uri = (%p)'%s'\n", localname, prefix, URI, URI );
      for ( int indexNamespace = 0; indexNamespace < nb_namespaces; ++indexNamespace )
      {
         const xmlChar *prefix = namespaces[indexNamespace*2];
         const xmlChar *nsURI = namespaces[indexNamespace*2+1];
         //printf( "  namespace: name='%s' uri=(%p)'%s'\n", prefix, nsURI, nsURI );
      }

      // if the element has attributes create an object otherwise store the name
      if(nb_attributes) {
        //printf("element: %s\n", localname);
        Local<Object> tmp = Object::New(xsr.isolate_);
        unsigned int index = 0;
        for ( int indexAttribute = 0;
              indexAttribute < nb_attributes;
              ++indexAttribute, index += 5 )
        {
           const xmlChar *localname = attributes[index];
           //const xmlChar *prefix = attributes[index+1];
           //const xmlChar *nsURI = attributes[index+2];
           const xmlChar *valueBegin = attributes[index+3];
           const xmlChar *valueEnd = attributes[index+4];
           std::string value( (const char *)valueBegin, (const char *)valueEnd );
           tmp->Set(String::NewFromUtf8(xsr.isolate_,(char*)localname), String::NewFromUtf8(xsr.isolate_,value.c_str()));
           //printf("attribute: %s value: %s\n", localname, value.c_str());
        }
        obj->Set(String::NewFromUtf8(xsr.isolate_,(char*)localname), tmp);
      } else {
        xsr.beginelem = (char*)localname;
        //printf("start: %s\n", xsr.beginelem);
        xsr.startelem = true;
      }
      xsr.inelem = true;
   }

   static void endElementNs( void * ctx,
                             const xmlChar * localname,
                             const xmlChar * prefix,
                             const xmlChar * URI )
   {
      Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      Local<Object> obj = xsr.ostack.back();
      xsr.ostack.pop_back();

      // if at the end of a wrapping element (last callback was also an endElement)
      if(!xsr.inelem) {
        int n = xsr.istack.size();
        Local<Object> tmp = xsr.istack.back();
        xsr.istack.pop_back();
        Local<Array> p = obj->GetPropertyNames();
        v8::String::Utf8Value utfname(p->Get(p->Length()-1)->ToString());
        string strname(*utfname);
        if(n == 1) {
          printf("here3\n");
          // there is just an element no array
          //printf("element: %s\n", xsr.beginelem);
          //printf("name: %s\n", strname.c_str());
          if(strname == "undefined") {
            printf("%i\n", xsr.estack.size());
            obj->Set(String::NewFromUtf8(xsr.isolate_,(char*)localname), tmp);
            for(Local<Object> ob : xsr.estack) {
              Local<Array> p3 = ob->GetPropertyNames();
              v8::String::Utf8Value utfname3(p3->Get(0)->ToString());
              string strname3(*utfname3);
              Local<Object> tmp = Object::New(xsr.isolate_);
              tmp->Set(p3->Get(0)->ToString(), ob);
              obj->Set(p3->Get(0)->ToString(), tmp);
            }
          } else {
            printf("here5\n");
            Local<Object> root = obj->Get(String::NewFromUtf8(xsr.isolate_,strname.c_str()))->ToObject();
            root->Set(String::NewFromUtf8(xsr.isolate_,xsr.beginelem), tmp);
          }
        } else if(n > 1) {
printf("here2\n");
          bool arr = true;
          string lastelem = "";
          for(int i = 1; i < n; i++) {
            Local<Object> ob = xsr.istack[i];
            Local<Array> obp = ob->GetPropertyNames();
            v8::String::Utf8Value obn(obp->Get(0)->ToString());
            string obs(*obn);
            //printf("obs: %s\n", obs.c_str());
            if(i > 1 && obs != lastelem)
              arr = false;
            lastelem = obs;
          }

          if(arr) {
            printf("here6\n");
          // get the array and unwrap each element's name
          Local<Array> array = Array::New(xsr.isolate_, n);
          Local<Object> tmpa = tmp;
          for(int i = n - 1; i >= 0; i--) {
            Local<Array> p2 = tmpa->GetPropertyNames();
            //v8::String::Utf8Value utfname2(p2->Get(0)->ToString());
            //string strname2(*utfname2);
            //printf("aname: %s\n", strname2.c_str());
            Local<Object> tmp2 = tmpa->Get(p2->Get(0)->ToString())->ToObject();
            //Local<Array> p3 = tmp2->GetPropertyNames();
            //v8::String::Utf8Value str2(p3->Get(0)->ToString());
            //string strname3(*str2);
            //Local<Object> avalue = tmp2->Get(p3->Get(0)->ToString())->ToObject();
            //Local<Array> p4 = avalue->GetPropertyNames();
            //v8::String::Utf8Value utfname3(p4->Get(0)->ToString());
            //string strname4(*utfname3);
            //printf("avalue: %s\n", strname3.c_str());
            /*Local<Object> tmp3 = tmp2->Get(p2->Get(0)->ToString())->ToObject();
            Local<Array> p4 = tmp3->GetPropertyNames();
            v8::String::Utf8Value str3(p4->Get(0)->ToString());
            string strname4(*str3);
            printf("avalue2: %s\n", strname4.c_str());*/
            array->Set(i, tmp2);
            tmpa = xsr.istack.back();
            xsr.istack.pop_back();
          }
          // add the array
          Local<Array> p2 = tmp->GetPropertyNames();
          v8::String::Utf8Value utfname2(p2->Get(0)->ToString());
          string strname2(*utfname2);
          obj->Set(String::NewFromUtf8(xsr.isolate_,strname2.c_str()), array);
        } else {
        Local<Object> tmpa = tmp;
        printf("here\n");
        Local<Object> root = obj->Get(String::NewFromUtf8(xsr.isolate_,(char*)localname))->ToObject();
        xsr.estack.push_back(root);
        for(int i = 0; i < n - 1; i++) {
          Local<Array> p2 = tmpa->GetPropertyNames();
          v8::String::Utf8Value utfname2(p2->Get(p2->Length() - 1)->ToString());
          string strname2(*utfname2);
          Local<String> value = tmpa->Get(p2->Get(p2->Length() - 1)->ToString())->ToString();
          //Local<Object> tmp2 = tmpa->Get(p2->Get(0)->ToString())->ToObject();
          //obj->Set(String::NewFromUtf8(xsr.isolate_,strname2.c_str()), tmpa);
          Local<Array> p3 = root->GetPropertyNames();
          v8::String::Utf8Value utfname3(p3->Get(0)->ToString());
          string strname3(*utfname3);
          printf("root: %s", strname3.c_str());
          root->Set(String::NewFromUtf8(xsr.isolate_,strname2.c_str()), value);
          printf("%i localname %s aname: %s beginelem: %s\n", i, (char*) localname, strname2.c_str(), xsr.beginelem);
          tmpa = xsr.istack.back();
          xsr.istack.pop_back();
        }
      }
        xsr.istack.clear();
        //for(Local<Object> ob : tmpv)
        //  xsr.istack.push_back(ob);
      }
    }

      if(!xsr.buffer.empty()) {
        /*if(strname == "undefined") {
          printf("elem: %s\n", xsr.beginelem);
          Local<Object> tmp = Object::New(xsr.isolate_);
          tmp->Set(String::NewFromUtf8(xsr.isolate_,xsr.beginelem), String::NewFromUtf8(xsr.isolate_,xsr.buffer.c_str()));
          obj->Set(String::NewFromUtf8(xsr.isolate_,xsr.beginelem), tmp);
          //xsr.istack.push_back(tmp);
        } else {*/
          //printf("name2: %s\n", (char*)localname);
          //Local<Object> tmp2 = obj->Get();
          obj->Set(String::NewFromUtf8(xsr.isolate_,(char*)localname),  String::NewFromUtf8(xsr.isolate_,xsr.buffer.c_str()));
          xsr.buffer.clear();
        //}
        //xsr.istack.push_back(obj);
      }

      xsr.istack.push_back(obj);
      xsr.object_ = obj;
      xsr.inelem = false;
      xsr.startelem = false;
   }

   static void error( void * ctx,
                      const char * msg,
                      ... )
   {
      //Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   static void warning( void * ctx,
                        const char * msg,
                        ... )
   {
      //Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   Local<Object> getObject() { return this->object_; }

private:

   // return Object
   Local<Object> object_;
   Isolate* isolate_;
   // object stack
   vector<Local<Object>> ostack;
   vector<Local<Object>> istack;
   bool inelem;
   char* beginelem;
   bool startelem;
   char* elem;
   string buffer;
   vector<Local<Object>> estack;
};

void parse(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<Object> bufferObj = args[0]->ToObject();
  char* bufferData   = Buffer::Data(bufferObj);
  size_t bufferLength = Buffer::Length(bufferObj);

  xmlSAXHandler saxHandler;
  memset( &saxHandler, 0, sizeof(saxHandler) );
  saxHandler.initialized = XML_SAX2_MAGIC;
  saxHandler.characters = &Xmlsonator::characters;
  saxHandler.startElementNs = &Xmlsonator::startElementNs;
  saxHandler.endElementNs = &Xmlsonator::endElementNs;
  saxHandler.warning = &Xmlsonator::warning;
  saxHandler.error = &Xmlsonator::error;

  Xmlsonator xsr(Object::New(isolate), isolate);
  int result = xmlSAXUserParseMemory( &saxHandler, &xsr, bufferData, int(bufferLength) );
  if ( result != 0 )
  {
    printf("Failed to parse document.\n" );
  } else
    args.GetReturnValue().Set(xsr.getObject());

  xmlCleanupParser();

  xmlMemoryDump();

}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "parseXML", parse);
}

NODE_MODULE(xmlsonator, init)
