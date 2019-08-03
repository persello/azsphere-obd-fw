#pragma once

int startCommandInterpreter(int (*_receive)(char**), int (*_send)(char*));

int stopCommandInterpreter();