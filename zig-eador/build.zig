const std = @import("std");

// zig build -Dgog_exe=<GOG installer .exe>             unpack + import assets + build
// zig build -Dgog_game=<installed "Eador - Genesis">    import assets + build
// zig build run                                         run with zig-out/assets
// zig build test                                        unit tests
// zig build test -Dforce                                rerun unit tests ignoring the cache
//
// Assets go to zig-out/assets/<lang>/ and zig-out/assets/common/; the
// language comes from the installer's goggame-*.info.  Importing a second
// language adds zig-out/assets/<lang>/ and leaves common/ unchanged.
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const gog_exe = b.option([]const u8, "gog_exe", "GOG installer (setup_eador_genesis_*.exe) to unpack and import into zig-out/assets");
    const gog_game = b.option([]const u8, "gog_game", "Installed GOG copy of Eador: Genesis to import into zig-out/assets");
    const force = b.option(bool, "force", "Rerun tests even if the cached result is up to date") orelse false;

    const raylib_dep = b.dependency("raylib_zig", .{ .target = target, .optimize = optimize });

    const mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    mod.addImport("raylib", raylib_dep.module("raylib"));
    const exe = b.addExecutable(.{ .name = "eador", .root_module = mod });
    b.installArtifact(exe);

    // The importer decodes ~4200 bitmaps; build it optimised even in Debug.
    const imp_mod = b.createModule(.{
        .root_source_file = b.path("src/import.zig"),
        .target = target,
        .optimize = if (optimize == .debug) .safe else optimize,
    });
    const imp = b.addExecutable(.{ .name = "eador-import", .root_module = imp_mod });
    b.installArtifact(imp);

    const import_step = b.step("import", "Import assets from -Dgog_exe=<installer> or -Dgog_game=<dir> into zig-out/assets");
    if (gog_exe != null and gog_game != null) {
        import_step.dependOn(&b.addFail("pass either -Dgog_exe or -Dgog_game, not both").step);
    } else if (gog_exe orelse gog_game) |src| {
        // Relative paths are relative to where `zig build` was invoked.
        const abs = if (std.fs.path.isAbsolute(src)) src else blk: {
            const cwd = std.process.currentPathAlloc(b.graph.io, b.graph.arena) catch @panic("cwd");
            break :blk b.pathResolve(&.{ cwd, src });
        };
        const run_imp = b.addRunArtifact(imp);
        // Writes into zig-out/assets in place (unchanged files are kept), so
        // it is not a cached output directory.
        run_imp.stdio = .inherit;
        run_imp.addArg(if (gog_exe != null) "--exe" else "--game");
        run_imp.addArg(abs);
        run_imp.addDirectoryArg2(b.graph.path(.install_prefix, "assets"), .{ .make_absolute = true });
        import_step.dependOn(&run_imp.step);
        b.getInstallStep().dependOn(&run_imp.step);
    } else {
        import_step.dependOn(&b.addFail("pass -Dgog_exe=<setup_eador_genesis_*.exe> or -Dgog_game=\"/path/to/Eador - Genesis\"").step);
    }

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    run_cmd.addDirectoryArg(b.graph.path(.install_prefix, "assets"));
    b.step("run", "Run the game (uses zig-out/assets)").dependOn(&run_cmd.step);

    const test_mod = b.createModule(.{
        .root_source_file = b.path("src/tests.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    test_mod.addImport("raylib", raylib_dep.module("raylib"));
    const tests = b.addTest(.{ .root_module = test_mod });
    const run_tests = b.addRunArtifact(tests);
    run_tests.has_side_effects = force;
    b.step("test", "Run unit tests").dependOn(&run_tests.step);
}
