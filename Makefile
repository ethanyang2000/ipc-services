CXX = g++
SRC_DIR = src
BIN_DIR = bin
SNAPPY_DIR = snappy-c
INCDIRS = -I ${SNAPPY_DIR}
CXXFLAGS = ${INCDIRS} -ggdb -Wall -std=c++11
LDFLAGS = -L${SNAPPY_DIR} snappy-c/snappy.h -l:libsnappyc.so.1

SRC_CLIENT = ${SRC_DIR}/tinylib.cpp
SRC_SERVER = ${SRC_DIR}/service.cpp
SRC_APP = ${SRC_DIR}/app.cpp

OBJ_DIR = ${BIN_DIR}/obj
OBJ_CLIENT = ${OBJ_DIR}/tinylib.o
OBJ_SERVER = ${OBJ_DIR}/service.o
OBJ_APP = ${OBJ_DIR}/app.o

all: snappy server app

${OBJ_DIR}/tinylib.o: ${SRC_CLIENT} | ${OBJ_DIR}
	$(CXX) $(CXXFLAGS) -c $< -o $@

${OBJ_DIR}/service.o: ${SRC_SERVER} | ${OBJ_DIR}
	$(CXX) $(CXXFLAGS) -c $< -o $@

${OBJ_DIR}/app.o: ${SRC_APP} | ${OBJ_DIR}
	$(CXX) $(CXXFLAGS) -c $< -o $@

${BIN_DIR}/service: ${OBJ_SERVER} | ${BIN_DIR}
	$(CXX) $(CXXFLAGS) $^ -o $@ ${LDFLAGS}

${BIN_DIR}/app: ${OBJ_APP} ${OUT} | ${BIN_DIR}
	$(CXX) $(CXXFLAGS) $^ -o $@ ${LDFLAGS}

snappy:
	$(MAKE) -C ${SNAPPY_DIR}

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

${BIN_DIR}:
	mkdir -p ${BIN_DIR}

service: ${OBJ_SERVER} | ${BIN_DIR}
	$(CXX) $(CXXFLAGS) $^ -o ${BIN_DIR}/service ${LDFLAGS}

app: ${OBJ_APP} | ${BIN_DIR}
	$(CXX) $(CXXFLAGS) ${OBJ_APP} -o ${BIN_DIR}/app ${LDFLAGS}

clean:
	-@rm -rf ${OBJ_DIR} ${BIN_DIR}/*.a ${BIN_DIR}/server ${BIN_DIR}/app
	@echo Cleaned!
