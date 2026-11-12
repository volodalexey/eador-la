//! The hero's quest list: dcHero::CheckQuestDone (Hero.cpp:2750) and
//! dcHero::AbandonQuest (Hero.cpp:2831).
//!
//! CheckQuestDone only sets bComplete: the encounter of a done quest
//! (ipEncDone, CallEncounter) is not ported.
const std = @import("std");
const Proto = @import("../data/proto.zig").Proto;
const hero_mod = @import("hero.zig");
const Hero = hero_mod.Hero;
const province = @import("province.zig");

/// Quest types (clQuest::iType) that need the quest's site.
pub const kind_bring_item = 7;
pub const kind_kill_site = 8;
pub const kind_have_item = 9;

/// arSite[ipSite] of the quest's province, null when it has none.
pub fn site(provinces: []province.Province, q: hero_mod.Quest) ?*province.Site {
    if (q.province <= 0 or q.province >= provinces.len) return null;
    if (q.site < 0 or q.site >= province.max_site) return null;
    return &provinces[@intCast(q.province)].sites[@intCast(q.site)];
}

fn hasItem(h: *const Hero, item: i32) bool {
    for (h.inventory[1..]) |it| if (it[0] == item) return true;
    return false;
}

/// CheckQuestDone(i).
pub fn checkDone(pr: *const Proto, provinces: []province.Province, h: *Hero, i: usize) void {
    var q = h.quest(i);
    if (q.index <= 0 or q.index >= pr.quests.len) return;
    const qp = &pr.quests[@intCast(q.index)];
    if (!q.complete and qp.kind < kind_bring_item) {
        var done = true;
        for (qp.tasks[0..hero_mod.quest_tasks], 0..) |task, j| {
            if (task.index > 0 and q.progress[j] < task.quantity) done = false;
        }
        if (done) q.complete = true;
    }
    if (qp.kind == kind_bring_item) {
        q.complete = if (site(provinces, q)) |st| hasItem(h, st.data) else false;
    }
    if (qp.kind == kind_have_item) q.complete = hasItem(h, qp.tasks[0].param);
    if (qp.kind == kind_kill_site and !q.complete) {
        if (site(provinces, q)) |st| if (st.data <= q.progress[0]) {
            q.complete = true;
        };
    }
    h.setQuest(i, q);
}

/// AbandonQuest(i): a site that gave the quest offers it again (arSite[3]
/// changes sign), the later quests move up.
pub fn abandon(pr: *const Proto, provinces: []province.Province, h: *Hero, i: usize) void {
    const q = h.quest(i);
    if (q.index > 0 and q.index < pr.quests.len and pr.quests[@intCast(q.index)].giver > 0) {
        if (site(provinces, q)) |st| st.quest = -st.quest;
    }
    for (i..hero_mod.quest_limit - 1) |j| h.setQuest(j, h.quest(j + 1));
    var last = h.quest(hero_mod.quest_limit - 1);
    last.index = 0;
    h.setQuest(hero_mod.quest_limit - 1, last);
}
