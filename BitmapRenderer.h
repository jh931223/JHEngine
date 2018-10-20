#pragma once
#include "Component.h"
#include "BitmapClass.h"
class Mesh;
class Material;
class BitmapRenderer :
	public Component
{
private:

	Material * material;
	BitmapClass* bitmap;
public:
	BitmapRenderer();
	~BitmapRenderer();
	Material* GetMaterial();
	void SetMaterial(Material* _material);
	void SetBitmap(int width, int height);
	void Render(ID3D11DeviceContext * _deviceContext, XMMATRIX _view, XMMATRIX _proj);
};

