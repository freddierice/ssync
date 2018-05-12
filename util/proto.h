#ifndef __PROTO_H__
#define __PROTO_H__
#include <atomic>
#include <memory>

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>

#include "util/exception.h"

namespace ssync {
namespace util {

	SSYNC_EXCEPTION(SSyncException, ProtoException);

	class Proto {
	public:
		Proto(int fd);
		~Proto();
		
		// send/recv a protobuf, recv returns true if a message is returned.
		void sendMessage(google::protobuf::Message& message);
		bool recvMessage(google::protobuf::Message& message);

		// parseVarint is pretty much copy-pastad from protobuf coded_stream.cc
		// I've included it here because it is not exported.
		static bool readVarint(void *data, int size, uint64_t *value);

		// true while the connection is valid.
		bool connected();

	private:
		// no default constructor, only valid with connection.
		Proto() = delete;

		// disconnect and throw
		[[ noreturn]] void disconnect(const std::string& msg);

		int m_fd;
		uint32_t m_recvBufferLen, m_sendBufferLen;
		char *m_recvBuffer, *m_sendBuffer;
		char *m_recvBufferIter;
		uint32_t m_recvLen;
		std::atomic<bool> m_connected;
	};
}
}

#endif /* __PROTO_H__ */
