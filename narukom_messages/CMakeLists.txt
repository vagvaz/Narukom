message(STATUS "Narukom Messages")

add_library(narukom_messages SHARED Envelope.pb.cc Nack.pb.cc NetworkPacket.pb.cc ChannelCommand.pb.cc)
