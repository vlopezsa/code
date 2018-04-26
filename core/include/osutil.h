#pragma once

#include "scene.h"

bool osOpenDlg(char *outFileName, size_t maxLength);

void osDisplaySceneInfo(Scene *s, bool extended = false);
