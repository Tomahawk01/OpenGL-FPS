#pragma once

namespace Game::Version {
    
    static constexpr auto MAJOR = 0;
    static constexpr auto MINOR = 0;
    static constexpr auto PATCH = 9;

}

namespace Game::Config {

#if DEBUG
    static constexpr bool logToFile = true;
#else
    static constexpr bool logToFile = false;
#endif

    static constexpr bool loggingEnabled = true;
    static constexpr bool openGLDebugEnabled = true;

}
