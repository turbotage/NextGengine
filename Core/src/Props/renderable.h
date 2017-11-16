#pragma once

#include "../def.h"

template <typename T>
class Renderable
{
private:
	uint8 m_RenderProps;
	T* m_DerivedParent;
public:

	enum eRenderProps {
		RENDERING_ON = 1,
	};

	Renderable();
	Renderable(uint8 renderProps);
protected:
	Renderable(T* derivedParent);
	Renderable(uint8 renderProps, T* derivedParent);
};

template<typename T>
inline Renderable<T>::Renderable()
{
}

template<typename T>
inline Renderable<T>::Renderable(uint8 renderProps)
{
	m_RenderProps = renderProps;
}

template<typename T>
inline Renderable<T>::Renderable(T * derivedParent)
{
	m_DerivedParent = derivedParent;
}

template<typename T>
inline Renderable<T>::Renderable(uint8 renderProps, T * derivedParent)
{
	m_RenderProps = renderProps;
	m_DerivedParent = derivedParent;
}
