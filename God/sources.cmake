# Общие списки исходников для Linux и Android сборок
set(EADOR_GAME_SOURCES
  Audio.cpp BattleAI.cpp Battlefield.cpp BF_Unit.cpp Campaign.cpp Castle.cpp
  CurrentGameState.cpp Eador.cpp Event.cpp GlobalCrutches.cpp G_Object.cpp
  GameMenu.cpp Globals.cpp Hero.cpp Interface.cpp Keymaster.cpp Logging.cpp
  MainAI.cpp MainProcess.cpp Mapper.cpp Player.cpp Prototype.cpp Province.cpp
  ProvList.cpp Randomizer.cpp Screens.cpp Simulation.cpp Unit.cpp Windower.cpp
  WorldRules.cpp Yuplay.cpp
)

set(EADOR_PLATFORM_SOURCES
  allegro_rl.cpp
  dat_loader.cpp
  pakfile_rl.cpp
  rl_backend.cpp
  NativeDisplay_rl.cpp
)
