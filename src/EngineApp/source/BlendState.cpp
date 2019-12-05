#include "pch.h"

#include <boost/container_hash/hash.hpp>

#include "BlendState.h"

BlendState::BlendState()
	: enabled(false)
	, srcColor(BlendFactor::ONE)
	, srcAlpha(BlendFactor::ONE)
	, dstColor(BlendFactor::ZERO)
	, dstAlpha(BlendFactor::ZERO)
	, op(BlendOp::ADD)
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
	: enabled(enabled)
	, srcColor(srcColor)
	, srcAlpha(srcAlpha)
	, dstColor(dstColor)
	, dstAlpha(dstAlpha)
	, op(op)
	//, m_hash(0)
{
	//boost::hash_combine(m_hash, m_enabled);
	//boost::hash_combine(m_hash, m_srcColor);
	//boost::hash_combine(m_hash, m_srcAlpha);
	//boost::hash_combine(m_hash, m_dstColor);
	//boost::hash_combine(m_hash, m_dstAlpha);
	//boost::hash_combine(m_hash, m_op);
}

BlendState::BlendState(bool enabled, BlendFactor srcFactor, BlendFactor dstFactor, BlendOp op)
	: enabled(enabled)
	, srcColor(srcFactor)
	, srcAlpha(srcFactor)
	, dstColor(dstFactor)
	, dstAlpha(dstFactor)
	, op(op)
	//, m_hash(0)
{
	//boost::hash_combine(m_hash, m_enabled);
	//boost::hash_combine(m_hash, m_srcColor);
	//boost::hash_combine(m_hash, m_srcAlpha);
	//boost::hash_combine(m_hash, m_dstColor);
	//boost::hash_combine(m_hash, m_dstAlpha);
	//boost::hash_combine(m_hash, m_op);
}

BlendState::BlendState(const BlendState& other) 
	: enabled(other.enabled)
	, srcColor(other.srcColor)
	, srcAlpha(other.srcAlpha)
	, dstColor(other.dstColor)
	, dstAlpha(other.dstAlpha)
	, op(other.op)
	//, m_hash(other.m_hash)
{}