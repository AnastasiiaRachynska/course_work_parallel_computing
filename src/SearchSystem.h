//
// Created by User on 15.10.2026.
//

#ifndef COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H
#define COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H

#include "Ollama.h"

class SearchSystem {
public:
    Ollama& ollama;
    SearchSystem(Ollama& ollama);
}; // SearchSystem

#endif //COURSE_WORK_PARALLEL_COMPUTING_SEARCHSYSTEM_H