// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: metric.proto

#include "metric.pb.h"
#include "metric.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>

static const char* MetricManager_method_names[] = {
  "/MetricManager/MetricPublish",
};

std::unique_ptr< MetricManager::Stub> MetricManager::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< MetricManager::Stub> stub(new MetricManager::Stub(channel, options));
  return stub;
}

MetricManager::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_MetricPublish_(MetricManager_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status MetricManager::Stub::MetricPublish(::grpc::ClientContext* context, const ::MetricPublishRequest& request, ::MetricPublishResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::MetricPublishRequest, ::MetricPublishResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_MetricPublish_, context, request, response);
}

void MetricManager::Stub::experimental_async::MetricPublish(::grpc::ClientContext* context, const ::MetricPublishRequest* request, ::MetricPublishResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::MetricPublishRequest, ::MetricPublishResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_MetricPublish_, context, request, response, std::move(f));
}

void MetricManager::Stub::experimental_async::MetricPublish(::grpc::ClientContext* context, const ::MetricPublishRequest* request, ::MetricPublishResponse* response, ::grpc::experimental::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_MetricPublish_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::MetricPublishResponse>* MetricManager::Stub::PrepareAsyncMetricPublishRaw(::grpc::ClientContext* context, const ::MetricPublishRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::MetricPublishResponse, ::MetricPublishRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_MetricPublish_, context, request);
}

::grpc::ClientAsyncResponseReader< ::MetricPublishResponse>* MetricManager::Stub::AsyncMetricPublishRaw(::grpc::ClientContext* context, const ::MetricPublishRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncMetricPublishRaw(context, request, cq);
  result->StartCall();
  return result;
}

MetricManager::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      MetricManager_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< MetricManager::Service, ::MetricPublishRequest, ::MetricPublishResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](MetricManager::Service* service,
             ::grpc::ServerContext* ctx,
             const ::MetricPublishRequest* req,
             ::MetricPublishResponse* resp) {
               return service->MetricPublish(ctx, req, resp);
             }, this)));
}

MetricManager::Service::~Service() {
}

::grpc::Status MetricManager::Service::MetricPublish(::grpc::ServerContext* context, const ::MetricPublishRequest* request, ::MetricPublishResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


