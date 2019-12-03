#pragma once

struct BlendFactor
{
	enum
	{
		ONE,
		ZERO,
		SRC_ALPHA,
		INV_SRC_ALPHA,
		DST_ALPHA,
		INV_DEST_ALPHA,
		SRC_COLOR,
		INV_SRC_COLOR
	};
};

struct BlendOp
{
	enum
	{
		ADD,
		SUBSTRACT
	};
};

class BlendState
{
public:
	BlendState();
	BlendState(bool enabled, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, BlendOp op);
	BlendState(bool enabled, BlendFactor srcFactor, BlendFactor dstFactor, BlendOp op);
	BlendState(const BlendState& other);

	bool IsTransparent() const;
	size_t GetHash() const;

private:
	bool m_enabled;
	BlendFactor m_srcColor, m_dstColor;
	BlendFactor m_srcAlpha, m_dstAlpha;
	BlendOp m_op;
	size_t m_hash;
};