AM_LDFLAGS = $(PROTOBUF_LIBS)

dist_noinst_DATA = *.proto

CLEANFILES = *.pb.cc *.pb.h
BUILT_SOURCES = test.pb.cc

bin_PROGRAMS = test
test_SOURCES = test.cpp test.pb.cc

%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=./ --cpp_out=./ ./$<

