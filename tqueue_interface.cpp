#include "tqueue_interface.h"
#include "tqueue.hpp"
extern "C"{
    QHandle create_tqueue(){ return new TQueue<>()}
}