#pragma once

#include <vector>

namespace d3drender
{
	class Stencil
	{
	public:
		Stencil(unsigned int width, unsigned int height);

		Stencil(const Stencil&) = delete;
		Stencil& operator=(const Stencil&) = delete;

		void Set(unsigned int i, unsigned int j);
		bool Get(unsigned int i, unsigned int j) const;
	private:

		size_t offset(unsigned int i, unsigned int j) const;

	private:
		std::vector<bool> m_mask;
		const unsigned int m_width;
		const unsigned int m_height;
	};

	inline Stencil::Stencil(unsigned int width, unsigned int height)
		: m_width(width)
		, m_height(height)
	{
		m_mask.resize(width * height, false);
	}

	inline void Stencil::Set(unsigned int i, unsigned int j)
	{
		auto off = offset(i, j);
		if (off < m_mask.size())
		{
			m_mask[off] = true;
		}
	}

	inline bool Stencil::Get(unsigned int i, unsigned int j) const
	{
		auto off = offset(i, j);
		if (off < m_mask.size())
		{
			return m_mask[off];
		}
		return false;
	}

	inline size_t Stencil::offset(unsigned int i, unsigned int j) const
	{
		auto off = j * m_width + i;
		assert(off < m_mask.size());
	}
}