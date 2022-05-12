/*
 * Copyright (c) 2022, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 *
 */

//!
//! \file     KeyEvent.h
//! \brief    This is the key event for the application.
//!

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <limits.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <memory.h>

class KeyEvent
{
private:
    static int m_qPressed;

    static struct termios m_tty;

    static volatile int m_received_sigterm;

    void Key_term_init(void);

    static void Key_term_exit(void);

    static void Key_sigterm_handler(int signal);

    int Key_read(void);

public:
    KeyEvent(/* args */);

    virtual ~KeyEvent() = default;

    bool Is_quit();

};

int KeyEvent::m_qPressed = 0;

struct termios KeyEvent::m_tty;

volatile int KeyEvent::m_received_sigterm = 0;

KeyEvent::KeyEvent(/* args */)
{
    m_qPressed = 0;
    memset(&m_tty, 0, sizeof(m_tty));
    m_received_sigterm = 0;
}

void KeyEvent::Key_term_init()
{

    struct termios tty;

    tcgetattr(0, &tty);
    m_tty = tty;
    atexit(Key_term_exit);

    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                |INLCR|IGNCR|ICRNL|IXON);
    tty.c_oflag |= OPOST;
    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
    tty.c_cflag &= ~(CSIZE|PARENB);
    tty.c_cflag |= CS8;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    tcsetattr (0, TCSANOW, &tty);

    signal(SIGQUIT, Key_sigterm_handler);

    signal(SIGINT , Key_sigterm_handler);
    signal(SIGTERM, Key_sigterm_handler);
}

void KeyEvent::Key_term_exit()
{
    tcsetattr(0, TCSANOW, &m_tty);
}

void KeyEvent::Key_sigterm_handler(int signal)
{
    m_received_sigterm = signal;
    m_qPressed++;
    Key_term_exit();
}

int KeyEvent::Key_read(void)
{
    int number_read = 1;
    unsigned char input_char;
    struct timeval tv;
    fd_set read_fds;

    FD_ZERO(&read_fds);
    FD_SET(0, &read_fds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    number_read = select(1, &read_fds, NULL, NULL, &tv);

    if (number_read > 0)
    {
        number_read = read(0, &input_char, 1);

        if (number_read == 1)
         return input_char;

        return number_read;
    }

    return -1;
}

bool KeyEvent::Is_quit()
{
    int key = 0;
    static bool bFirst = true;
    if (bFirst) {
        Key_term_init();
    }
    bFirst = false;

    if (m_qPressed)
        return true;

    key = Key_read();
    if (key == 'q')
    {
        printf("press [q] to quit\n");
        return true;
    }
    return false;
}