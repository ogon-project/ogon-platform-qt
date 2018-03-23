TEMPLATE = lib
CONFIG += staticlib
TARGET = qsbp

include(../protobuf.pri)
isEmpty(SBP_PROTO_PREFIX) {
	packagesExist(ogon-backend1) {
		PROTOFILE_PREFIX=$$system(pkg-config ogon1 --variable=protocol_dir)
	} else {
		include(../global.pri)
		PROTOFILE_PREFIX=$$QOGON_PREFIX/share/ogon
	}
	PROTOS += $$PROTOFILE_PREFIX/protobuf/SBP.proto
} else {
		PROTOFILE_PREFIX=$$SBP_PROTO_PREFIX
		PROTOS += $$SBP_PROTO_PREFIX/SBP.proto
}

