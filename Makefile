## Define Compilers
CXX:=g++
CXX_FLAGS:=-Wall -std=c++17 -O0 -g # -O2
LIBS:=-lpthread -lssh2 -lprotobuf -lstdc++fs

## Gather the obj files
PROTOS=$(patsubst proto/%.proto,proto/%.pb.h,$(wildcard proto/*.proto))
PROTO_OBJS=$(patsubst proto/%.proto,proto/%.pb.o,$(wildcard proto/*.proto))

OBJS=$(patsubst %.cc,%.o,$(wildcard */*.cc)) $(PROTO_OBJS)

## Define targets
all: ssync ssyncd

## Define building procedure
ssync: proto $(OBJS) ssync.o
	$(CXX) $(CXX_FLAGS) -I. -o $@ ssync.o $(OBJS) $(LIBS)

ssyncd: proto $(OBJS) ssyncd.o
	$(CXX) $(CXX_FLAGS) -I. -o $@ ssyncd.o $(OBJS) $(LIBS)

# Generic dependencies
%.cc: %.h
proto/%.pb.o: proto/%.pb.cc proto/%.pb.h

proto/%.pb.cc proto/%.pb.h: proto/%.proto
	protoc -I=proto --cpp_out=proto proto/*.proto

%.o: %.cc
	$(CXX) $(CXX_FLAGS) -I. -c $^ -o $@


clean:
	@rm -f $(OBJS) proto/*.pb.* ssync.o ssyncd.o
