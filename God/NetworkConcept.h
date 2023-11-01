#pragma once

/*

interface Snapshot {
	poly_const_iterator<std::byte> begin() const;
	poly_const_iterator<std::byte> end() const;
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
