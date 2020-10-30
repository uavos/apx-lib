/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <apx.h>


struct test_s
{
  float v;
};

template <typename T>
struct tmpl_s
{
  T value;
};

class Test
{
public:
  explicit Test()
  {
  }
};


int main()
{
  //char *buf;

  struct test_s t;

  Test test;

  memcpy(&t,&test,sizeof(t));
  
  

  t.v = (1);

  log("Hello world!\n");

  for (unsigned int i = 1; i > 0; ++i)
  {
    t.v += 0.1f;
  }
  log("RESULT: %u\n", t.v);
  log("RESULT: %f\n", t.v);

  /*buf = malloc(1024);
    if (!buf) {
        printf("malloc buf failedB\n");
        return -1;
    }

    printf("buf ptr: %p\n", buf);

    sprintf(buf, "%s", "1234\n");
    printf("buf: %s", buf);

    free(buf);*/
    return 0;
}

EXPORT void hello()
{
  log("TEST!\n");
}

