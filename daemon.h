#ifndef DAEMON_H
#define DAEMON_H

/*
**	This header and the associated source file are heavily inspired by the examples from our course's book "The Linux Programming Interface"
**	I think many other people may have very similar results, but I GUARANTEE that I have never given anyone a copy of my work, nor have I copied anyone's work
*/

#include "DeLib/DeInO.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int become_daemon();
void release_lock();

#endif