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

using namespace std;
using namespace v8;
using namespace node;

class Xmlsonator
{
public:

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

      Local<Array> attr = Array::New(xsr.isolate_, nb_attributes);
      Local<Object> tmp = Object::New(xsr.isolate_);
      unsigned int index = 0;
      for ( int indexAttribute = 0;
            indexAttribute < nb_attributes;
            ++indexAttribute, index += 5 )
      {
         const xmlChar *localname = attributes[index];
         const xmlChar *prefix = attributes[index+1];
         const xmlChar *nsURI = attributes[index+2];
         const xmlChar *valueBegin = attributes[index+3];
         const xmlChar *valueEnd = attributes[index+4];
         std::string value( (const char *)valueBegin, (const char *)valueEnd );
         tmp->Set(String::NewFromUtf8(xsr.isolate_,(char*)localname), String::NewFromUtf8(xsr.isolate_,(char*)value.c_str()));
      }

      // if the element has attributes create an object otherwise store the name
      if(nb_attributes) {
        obj->Set(String::NewFromUtf8(xsr.isolate_,(char*)localname), tmp);
      } else {
        xsr.beginelem = (char*)localname;
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

      // if last callback was also an element end
      if(!xsr.inelem) {
        int n = xsr.istack.size();
        Local<Object> tmp = xsr.istack.back();
        xsr.istack.pop_back();
        Local<Array> p = obj->GetPropertyNames();
        v8::String::Utf8Value utfname(p->Get(p->Length())->ToString());
        string strname(*utfname);
        if(n == 1) {
          // there is just an element no array
          obj->Set(String::NewFromUtf8(xsr.isolate_,xsr.beginelem), tmp);
        } else if(n > 1) {
          // get the array and unwrap each element's name
          Local<Array> array = Array::New(xsr.isolate_, n);
          for(int i = 0; i < n; i++) {
            Local<Array> p2 = tmp->GetPropertyNames();
            v8::String::Utf8Value utfname2(p2->Get(0)->ToString());
            string strname2(*utfname2);
            Local<Object> tmp2 = tmp->Get(p2->Get(0)->ToString())->ToObject();
            Local<Array> p3 = tmp2->GetPropertyNames();
            v8::String::Utf8Value utfname3(p3->Get(0)->ToString());
            string strname3(*utfname3);
            array->Set(i, tmp2);
            Local<Object> tmp = xsr.istack.back();
            xsr.istack.pop_back();
          }
          // add the array
          Local<Array> p2 = tmp->GetPropertyNames();
          v8::String::Utf8Value utfname2(p2->Get(0)->ToString());
          string strname2(*utfname2);
          obj->Set(String::NewFromUtf8(xsr.isolate_,strname2.c_str()), array);
        }
        xsr.istack.clear();
      }

      xsr.istack.push_back(obj);
      xsr.object_ = obj;
      xsr.inelem = false;
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

   // return Object
   Local<Object> object_;
   Isolate* isolate_;
   // object stack
   vector<pair<char*, Local<Object>> > objs_;
   vector<Local<Object>> ostack;
   vector<Local<Object>> istack;
   bool inelem = false;
   char* beginelem;
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
  saxHandler.startElementNs = &Xmlsonator::startElementNs;
  saxHandler.endElementNs = &Xmlsonator::endElementNs;
  saxHandler.warning = &Xmlsonator::warning;
  saxHandler.error = &Xmlsonator::error;

  Xmlsonator xsr;
  xsr.object_ = Object::New(isolate);
  xsr.isolate_ = isolate;
  int result = xmlSAXUserParseMemory( &saxHandler, &xsr, bufferData, int(bufferLength) );
  if ( result != 0 )
  {
    printf("Failed to parse document.\n" );
  } else
  args.GetReturnValue().Set(xsr.object_);

  xmlCleanupParser();

  xmlMemoryDump();

}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "parseXML", parse);
}

NODE_MODULE(xmlsonator, init)
