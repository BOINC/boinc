all: sporadic

INC_DIRS = -I ../../api -I ../../lib
LIB_DIRS = -L ../../api -L ../../lib
CXX = g++ $(INC_DIRS)

sporadic: sporadic.cpp
	$(CXX) $(LIB_DIRS) sporadic.cpp -lboinc_api -lboinc -lpthread -o sporadic
