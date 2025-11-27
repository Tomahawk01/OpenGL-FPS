#pragma once

#include "KeyEvent.h"
#include "MouseEvent.h"
#include "MouseButtonEvent.h"
#include "StopEvent.h"

#include <variant>

namespace Game {

	using Event = std::variant<StopEvent, KeyEvent, MouseEvent, MouseButtonEvent>;

}
