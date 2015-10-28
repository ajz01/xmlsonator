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
      printf( "startElementNs: name = '%s' prefix = '%s' uri = (%p)'%s'\n", localname, prefix, URI, URI );
      for ( int indexNamespace = 0; indexNamespace < nb_namespaces; ++indexNamespace )
      {
         const xmlChar *prefix = namespaces[indexNamespace*2];
         const xmlChar *nsURI = namespaces[indexNamespace*2+1];
         printf( "  namespace: name='%s' uri=(%p)'%s'\n", prefix, nsURI, nsURI );
      }

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
         printf( "  %sattribute: localname='%s', prefix='%s', uri=(%p)'%s', value='%s'\n",
                 indexAttribute >= (nb_attributes - nb_defaulted) ? "defaulted " : "",
                 localname,
                 prefix,
                 nsURI,
                 nsURI,
                 value.c_str() );

        xsr.object_->Set(String::NewFromUtf8(xsr.isolate_,(char*)localname),String::NewFromUtf8(xsr.isolate_,value.c_str()));
      }
   }

   static void endElementNs( void * ctx,
                             const xmlChar * localname,
                             const xmlChar * prefix,
                             const xmlChar * URI )
   {
      Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      printf( "endElementNs: name = '%s' prefix = '%s' uri = '%s'\n", localname, prefix, URI );
   }

   static void error( void * ctx,
                      const char * msg,
                      ... )
   {
      Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   static void warning( void * ctx,
                        const char * msg,
                        ... )
   {
      Xmlsonator &xsr = *( static_cast<Xmlsonator *>( ctx ) );
      va_list args;
      va_start(args, msg);
      vprintf( msg, args );
      va_end(args);
   }

   Local<Object> object_;
   Isolate* isolate_;
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
   }

   xmlCleanupParser();

   xmlMemoryDump();

}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "parseXML", parse);
}

NODE_MODULE(xmlsonator, init)
