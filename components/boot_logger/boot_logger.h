#pragma once

#include "esphome/core/component.h"
#include <list>

namespace esphome {
namespace boot_logger {

class BootLogger : public Component {
   struct LogItem {
      int level;
      std::string tag;
      std::string message;
   };

   struct Config {
      std::uint32_t heap_limit;
      std::uint32_t dump_lines_per_loop;
      std::uint32_t connect_delay;
   };

   enum State {
      BUFFERING,
      DUMPING
   };

public:
   BootLogger(std::uint32_t heap_limit, std::uint32_t dump_lines_per_loop, std::uint32_t connect_delay);

   void setup() override;
   void loop() override;

private:

   void dumping();

   const Config m_config;
   State m_state{ State::BUFFERING };

   std::list<LogItem> m_items;
   uint32_t m_setup_ts {0};
   uint32_t m_connected_ts {0};
};

}
}
