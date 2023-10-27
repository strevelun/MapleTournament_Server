#pragma once

#define SINGLETON(name) \
private: \
static name* m_pInst; \
name(); \
~name(); \
public: \
	static name* GetInst() \
	{ \
		if (!m_pInst) m_pInst = new name; \
		return m_pInst; \
	} \
	static void DestroyInst() \
	{ \
		if (m_pInst) delete m_pInst; \
		m_pInst = nullptr; \
	} 