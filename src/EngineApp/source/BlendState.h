#pragma once

enum class BlendFactor
{
	ZERO,
	ONE,
	SRC_COLOR,
	INV_SRC_COLOR,
	SRC_ALPHA,
	INV_SRC_ALPHA,
	DST_ALPHA,
	INV_DST_ALPHA,
	DST_COLOR,
	INV_DST_COLOR,
};

enum class BlendOp
{
	ADD,
	SUBSTRACT
};

struct BlendState
{
	BlendState();
	BlendState(bool enabled, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, BlendOp op);
	BlendState(bool enabled, BlendFactor srcFactor, BlendFactor dstFactor, BlendOp op);
	BlendState(const BlendState& other);

	//bool IsTransparent() const;
	//size_t GetHash() const;

	bool enabled;
	BlendFactor srcColor, dstColor;
	BlendFactor srcAlpha, dstAlpha;
	BlendOp op;
	//size_t m_hash;
};