#include "pch.h"

#include <boost/container_hash/hash.hpp>

#include "BlendState.h"

BlendState::BlendState()
	//: m_enabled(false)
	//, m_srcColor(BlendFactor::ONE)
	//, m_srcAlpha(BlendFactor::ONE)
	//, m_dstColor(BlendFactor::ZERO)
	//, m_dstAlpha(BlendFactor::ZERO)
	//, m_op(BlendOp::ADD)
	//, m_hash(0)
{
	//boost::hash_combine(m_hash, m_enabled);
	//boost::hash_combine(m_hash, m_srcColor);
	//boost::hash_combine(m_hash, m_srcAlpha);
	//boost::hash_combine(m_hash, m_dstColor);
	//boost::hash_combine(m_hash, m_dstAlpha);
	//boost::hash_combine(m_hash, m_op);
}

BlendState::BlendState(bool enabled, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, BlendOp op)
	: m_enabled(enabled)
	, m_srcColor(srcColor)
	, m_srcAlpha(srcAlpha)
	, m_dstColor(dstColor)
	, m_dstAlpha(dstAlpha)
	, m_op(op)
	, m_hash(0)
{
	//boost::hash_combine(m_hash, m_enabled);
	//boost::hash_combine(m_hash, m_srcColor);
	//boost::hash_combine(m_hash, m_srcAlpha);
	//boost::hash_combine(m_hash, m_dstColor);
	//boost::hash_combine(m_hash, m_dstAlpha);
	//boost::hash_combine(m_hash, m_op);
}

BlendState::BlendState(bool enabled, BlendFactor srcFactor, BlendFactor dstFactor, BlendOp op)
	: m_enabled(enabled)
	, m_srcColor(srcFactor)
	, m_srcAlpha(srcFactor)
	, m_dstColor(dstFactor)
	, m_dstAlpha(dstFactor)
	, m_op(op)
	, m_hash(0)
{
	//boost::hash_combine(m_hash, m_enabled);
	//boost::hash_combine(m_hash, m_srcColor);
	//boost::hash_combine(m_hash, m_srcAlpha);
	//boost::hash_combine(m_hash, m_dstColor);
	//boost::hash_combine(m_hash, m_dstAlpha);
	//boost::hash_combine(m_hash, m_op);
}

BlendState::BlendState(const BlendState& other) 
	: m_enabled(other.m_enabled)
	, m_srcColor(other.m_srcColor)
	, m_srcAlpha(other.m_srcAlpha)
	, m_dstColor(other.m_dstColor)
	, m_dstAlpha(other.m_dstAlpha)
	, m_op(other.m_op)
	, m_hash(other.m_hash)
{}