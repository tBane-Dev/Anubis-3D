#pragma once

#include <SFML/OpenGL.hpp>
#include <SFML/Window/Context.hpp>
#include <cstddef>

namespace Anubis::GL {
	using GenBuffersFunction =
		void (APIENTRY*)(GLsizei, GLuint*);

	using BindBufferFunction =
		void (APIENTRY*)(GLenum, GLuint);

	using BufferDataFunction =
		void (APIENTRY*)(GLenum, std::ptrdiff_t, const void*, GLenum);

	using DeleteBuffersFunction =
		void (APIENTRY*)(GLsizei, const GLuint*);

	inline GenBuffersFunction genBuffers = nullptr;
	inline BindBufferFunction bindBuffer = nullptr;
	inline BufferDataFunction bufferData = nullptr;
	inline DeleteBuffersFunction deleteBuffers = nullptr;

	inline constexpr GLenum ArrayBuffer = 0x8892;
	inline constexpr GLenum ElementArrayBuffer = 0x8893;
	inline constexpr GLenum StaticDraw = 0x88E4;

	bool initialize();
}