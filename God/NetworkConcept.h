#pragma once

/*

interface Snapshot {
	std::string bytes() const;
}

interface Host {
	void send(Snapshot);
	Snapshot lastSnapshot() const;
}

interface State {
	Snapshot snapshot() const;
	void applay(Snapshot);
}

log = ForkedLog(
	StreamLog(std::cout),
	GameGlobalLog()
);
state = LoggedState(log, CurrentGameState());
life = LoggedHost(log, RemoteHost());

life.send(state.snapshot());
state.applay(life.lastSnapshot());

*/
