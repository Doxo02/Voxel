const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{ .preferred_optimize_mode = .ReleaseSmall });

    // ---------------------------
    // VoxelEngine static library
    // ---------------------------
    const engine = b.addLibrary(.{
        .name = "VoxelEngine",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libcpp = true,
            .link_libc = true,
            .sanitize_c = .off,
        }),
    });
    engine.root_module.addLibraryPath(.{ .cwd_relative = "/usr/lib64", });

    engine.root_module.addIncludePath(b.path("lib"));
    engine.root_module.addIncludePath(b.path("src"));
    engine.root_module.addSystemIncludePath(.{ .cwd_relative = "/usr/include" });

    const engine_sources = &[_][]const u8{
        "src/Engine/vxe/Application.cpp",
        "src/Engine/vxe/Platform/OpenGL/ogl_Shader.cpp",
        "src/Engine/vxe/Platform/OpenGL/ogl_VertexArray.cpp",
        "src/Engine/vxe/Platform/OpenGL/ogl_VertexBuffer.cpp",
        "src/Engine/vxe/Platform/OpenGL/ogl_IndexBuffer.cpp",
        "src/Engine/vxe/Platform/OpenGL/ogl_ShaderStorageBuffer.cpp",
        "src/Engine/vxe/Platform/OpenGL/ogl_RenderAPI.cpp",
        "src/Engine/vxe/Rendering/graphics/Factories.cpp",
        "src/Engine/vxe/Rendering/Renderer.cpp",
        "src/Engine/vxe/Rendering/VoxelGrid.cpp",
        "src/Engine/vxe/DataStructures/Grid.cpp",
        "src/Engine/vxe/DataStructures/BrickMap.cpp",
        "src/Engine/vxe/Core/Window.cpp",
        "src/Engine/vxe/Platform/Linux/LinuxWindow.cpp",
    };

    for (engine_sources) |src| {
        engine.root_module.addCSourceFile(.{ .file = b.path(src), .flags = &.{"-std=c++20"}, .language = .cpp });
    }

    engine.root_module.linkSystemLibrary("GL", .{});
    engine.root_module.linkSystemLibrary("GLEW", .{});
    engine.root_module.linkSystemLibrary("pthread", .{});


    // ---------------------------
    // VoxelApp executable
    // ---------------------------
    const exe = b.addExecutable(.{
        .name = "VoxelApp",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libcpp = true,
            .link_libc = true,
            .sanitize_c = .off,
        }),
        .use_llvm = true,
    });
    exe.root_module.addLibraryPath(.{ .cwd_relative = "/usr/lib64", });
    exe.root_module.addSystemIncludePath(.{ .cwd_relative = "/usr/include" });

    exe.root_module.addIncludePath(b.path("lib"));
    exe.root_module.addIncludePath(b.path("src/Engine"));
    exe.root_module.addIncludePath(b.path("third_party/imgui"));

    exe.root_module.addCSourceFile(.{ .file = b.path("src/App.cpp"), .flags = &.{"-std=c++20"}, .language = .cpp });
    exe.root_module.addCSourceFile(.{ .file = b.path("src/Rendering/Camera.cpp"), .flags = &.{"-std=c++20"}, .language = .cpp });

    exe.root_module.linkLibrary(engine);

    exe.root_module.linkSystemLibrary("GL", .{});
    exe.root_module.linkSystemLibrary("GLEW", .{});
    exe.root_module.linkSystemLibrary("glfw", .{});
    exe.root_module.linkSystemLibrary("pthread", .{});

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    const run_step = b.step("run", "Run the App");
    run_step.dependOn(&run_cmd.step);
}
