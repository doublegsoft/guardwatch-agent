/*
**           .d888
**          d88P"
**          888
**  .d88b.  888888 .d8888b
** d88P"88b 888   d88P"
** 888  888 888   888
** Y88b 888 888   Y88b.
**  "Y88888 888    "Y8888P
**      888
** Y8b d88P
**  "Y88P"
**
** Copyright (C) 2023 doublegsoft.open
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>

#include "gfc.h"


int
main()
{
  gfc_ring_p ring = gfc_ring_new(1000);

  int i = 0;
  int data[100];
  for (i = 0; i < 100; i++)
  {
    data[i] = i;
    gfc_ring_push(ring, &data[i]);
  }

  int* pi;
  for (i = 0; i < 100; i++)
  {
    pi = gfc_ring_pop(ring);
    printf("pop: %d\n", *pi);
  }

  gfc_ring_clear(ring);
  gfc_ring_free(ring);
  return 0;
}
