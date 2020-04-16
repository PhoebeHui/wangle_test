#include <iostream>
#include "wangle/channel/Pipeline.h"
#include "wangle/channel/Handler.h"
#include "wangle/channel/EventBaseHandler.h"
#include "wangle/codec/StringCodec.h"
#include "wangle/channel/AsyncSocketHandler.h"
#include "wangle/bootstrap/ClientBootstrap.h"
#include "wangle/codec/LineBasedFrameDecoder.h"
#include "folly/io/IOBufQueue.h"

typedef wangle::Pipeline<folly::IOBufQueue, std::string> MyPipeline;

class MyHandler : public wangle::HandlerAdapter < std::string>
{
public:
	void read(Context*, std::string msg) override {
		std::cout << "received back: " << msg;
	}
	void readException(Context* ctx, folly::exception_wrapper e) override {
		std::cout << folly::exceptionStr(e) << std::endl;
		close(ctx);
	}
	void readEOF(Context* ctx) override {
		std::cout << "EOF received :(" << std::endl;
		close(ctx);
	}
};

// chains the handlers together to define the response pipeline
class MyPipelineFactory : public wangle::PipelineFactory<MyPipeline> {
public:
	MyPipeline::Ptr newPipeline(
		std::shared_ptr<folly::AsyncTransportWrapper> sock) override {
		auto pipeline = MyPipeline::create();
		pipeline->addBack(wangle::AsyncSocketHandler(sock));
		pipeline->addBack(
			wangle::EventBaseHandler()); // ensure we can write from any thread
		pipeline->addBack(wangle::LineBasedFrameDecoder(8192, false));
		pipeline->addBack(wangle::StringCodec());
		pipeline->addBack(MyHandler());
		pipeline->finalize();
		return pipeline;
	}
};
class ClientServer
{
public:
	void run()
	{
		clientBootstrap_.group(std::make_shared < folly::IOThreadPoolExecutor>(1));
		clientBootstrap_.pipelineFactory(std::make_shared < MyPipelineFactory>());
		auto pipeline = clientBootstrap_.connect(folly::SocketAddress("127.0.0.1", 8800)).get();
		std::string line;
		pipeline->write("line").get();
		std::cout << "line " << line << std::endl;
	}
private:
	wangle::ClientBootstrap<MyPipeline> clientBootstrap_;
};


int main()
{
	ClientServer cs;
	cs.run();
	return 0;
}