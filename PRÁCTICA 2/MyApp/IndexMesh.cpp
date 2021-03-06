#include "IndexMesh.h"



IndexMesh::IndexMesh() : numIndices(0)
{
	type = GL_TRIANGLES;
}


IndexMesh::~IndexMesh()
{
	delete[] indices;
}

void IndexMesh::draw()
{
	enable();
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, indices);
	disable();
}


void IndexMesh::enable()
{
	Mesh::enable();
	if (indices != nullptr) {
		glEnableClientState(GL_INDEX_ARRAY);
		glIndexPointer(GL_UNSIGNED_INT, 0, indices);
	}
}

void IndexMesh::disable()
{
	Mesh::disable();
	glDisableClientState(GL_INDEX_ARRAY);
}
IndexMesh* IndexMesh::generateGrid(GLdouble lado, GLuint numDiv)
{
	numDiv = 256; // nVer=257.
	lado = numDiv * 8; // para incr=8
					  // grid cuadrado de lado*lado, centrado en el plano Y=0,
					  // dividido en numDiv*numDiv celdas
	IndexMesh* m = new IndexMesh();
	GLdouble incr = lado / numDiv; // incremento para la coordenada x, y la z
	GLuint numVer = numDiv + 1; // num. de v�rtices por filas y por columnas
								// generar v�rtices
	m->numVertices = numVer *numVer; // num. de v�rtices
	m->vertices = new glm::dvec3[m->numVertices];

	double z = -lado / 2;
	double x;
	for (int f = 0; f < numVer; f++)
	{
		x = -lado / 2;
		for (int c = 0; c < numVer; c++)
		{
			m->vertices[f *numVer + c] = glm::dvec3(x, 0, z);
			x += incr;
		}
		z += incr;
	}

	// generar �ndices
	m->numIndices = numDiv * numDiv  *6; ; // num. de �ndices
	m->indices = new GLuint[m->numIndices];

	int i = 0;
	GLuint vi;
	for (int f = 0; f < numDiv; f++)
	{
		for (int c = 0; c < numDiv; c++)
		{
			vi = f * numVer + c;
			m->indices[i++] = vi;
			m->indices[i++] = vi + numVer;
			m->indices[i++] = vi + 1;
			m->indices[i++] = vi + 1;
			m->indices[i++] = vi + numVer;
			m->indices[i++] = vi + numVer + 1;
		}
	}
	return m;
}

IndexMesh* IndexMesh::generateTerrain()
{
	std::ifstream file("..\\Bmps\\terrain.raw", std::ios::binary); // open
	if (!file.is_open()) return nullptr; // "terrain.raw": 257*257 unsigned chars
	GLuint nDiv = 256; // nVer=257.
	GLdouble lado = nDiv * 8; // para incr=8
	IndexMesh* m = generateGrid(lado, nDiv);
	unsigned char*  data = new unsigned char[m->numVertices];
	file.read((char*)data, m->numVertices * sizeof(char));// char = byte -> leer en data un bloque de numVertices bytes
	file.close();

	// modificar la coordenada Y de los v�rtices con los datos de data (*0.5)
	for (int i = 0; i < m->numVertices; i++)
	{
		m->vertices[i] = { m->vertices[i].x, data[i] *0.5, m->vertices[i].z };
	}

	// generar coordenadas de textura -> recorrido de v�rtices
	GLuint numVer = nDiv + 1;
	GLdouble incr = 1.0 / nDiv;

	m->texCoords = new glm::dvec2[m->numVertices];
	for (int f = 0; f < numVer; f++)
	{
		for (int c = 0; c < numVer; c++)
		{
			m->texCoords[f  *numVer + c] = glm::dvec2(f *incr, c *incr);
		}
	}
	delete data;
	// generar normales -> recorrido de tri�ngulos
	m->normals = new glm::dvec3[m->numVertices];
	for (int i = 0; i < m->numVertices; i++)
	{
		m->normals[i] = glm::dvec3(0.0, 0.0, 0.0);
	}

	for (int i = 0; i < m->numIndices; i += 3)
	{
		glm::dvec3 a = m->vertices[m->indices[i]];
		glm::dvec3 b = m->vertices[m->indices[i + 1]];
		glm::dvec3 c = m->vertices[m->indices[i + 2]];

		glm::dvec3 n = normalize(cross(b - a, c - a));

		m->normals[m->indices[i]] += n;
		m->normals[m->indices[i + 1]] += n;
		m->normals[m->indices[i + 2]] += n;
	}
	for (int i = 0; i < m->numVertices; i++)
	{
		m->normals[i] = normalize(m->normals[i]);
	}
	return m;
}
//IndexMesh* IndexMesh::generateGrid(GLdouble lado, GLuint numDiv)
//{
//	// grid cuadrado de lado*lado, centrado en el plano Y=0,
//  // dividido en numDiv*numDiv celdas
//	IndexMesh* m = new IndexMesh();
//	GLdouble incr = lado / numDiv; // incremento para la coordenada x, y la z
//	GLuint numVer = numDiv + 1; // num. de v�rtices por filas y por columnas
//								// generar v�rtices
//	m->numVertices = numVer * numVer; // num. de v�rtices
//	m->vertices = new glm::dvec3[m->numVertices];
//	GLdouble z, x;
//
//	z = -lado / 2;
//	for (int i = 0; i < numVer; i++) {
//		x = -lado / 2;
//		for (int j = 0; j < numVer; j++) {
//			m->vertices[i * numVer + j] = glm::dvec3(x, 0, z);
//			x += incr;
//		}
//		z += incr;
//	}
//	// generar �ndices
//	GLdouble iv;
//	m->numIndices = numDiv * numDiv * 6; ; // num. de �ndices
//	m->indices = new GLuint[m->numIndices];
//	int k = 0;
//	for (int i = 0; i < numDiv; i++) {
//		for (int j = 0; j < numDiv; j++) {
//			iv = i * numVer + j;
//			m->indices[k++] = iv;
//			m->indices[k++] = iv + numVer;
//			m->indices[k++] = iv + 1;
//			m->indices[k++] = iv + 1;
//			m->indices[k++] = iv + numVer;
//			m->indices[k++] = iv + numVer + 1;
//		}
//	}
//	return m;
//}
//IndexMesh* IndexMesh::generateTerrain() {
//	std::ifstream file("..//Bmps//terrain.raw", std::ios::binary); // open
//	if (!file.is_open()) return nullptr; // "terrain.raw": 257*257 unsigned chars
//	GLuint nDiv = 256; // nVer=257.
//	GLdouble lado = nDiv * 8; // para incr=8
//	GLuint nVer = nDiv + 1;
//	IndexMesh* m = generateGrid(lado, nDiv);
//	unsigned char * data = new unsigned char[m->numVertices];
//	file.read((char *)data, m->numVertices * sizeof(char));
//	file.close(); // char = byte -> leer en data un bloque de numVertices bytes
//
//	for (int i = 0; i < m->numVertices; i++) {
//		m->vertices[i].y = data[i];
//	}
//	// generar coordenadas de textura -> recorrido de v�rtices
//	m->texCoords = new glm::dvec2[m->numVertices];
//	GLdouble incr = 1 / nDiv;
//
//	for (int i = 0; i < nVer; i++) {
//		for (int j = 0; j < nVer; j++) {
//			m->texCoords[i * nVer + j]
//				= glm::dvec2(i*incr,j*incr);
//		}
//	}
//
//	m->normals = new glm::dvec3[m->numVertices];
//
//	for (int i = 0; i < m->numVertices; i++)
//		m->normals[i] = glm::dvec3(0, 0, 0);
//
//
//	for (int i = 0; i < m->numIndices; i+=3)
//	{
//		int a, b, c;
//
//		a = m->indices[i];
//		b = m->indices[i + 1];
//		c = m->indices[i + 2];
//
//		/*C�lculo del vector normal al tri�ngulo (a, b, c) : el producto
//		vectorial (c � b) x (a � b). */
//		glm::dvec3 n;
//		
//		n = glm::cross(m->vertices[b] - m->vertices[a], m->vertices[c] - m->vertices[a]);
//
//		m->normals[a] += n;
//		m->normals[b] += n;
//		m->normals[c] += n;
//	}
//
//	for (int i = 0; i < m->numVertices; i++)
//		glm::normalize(m->normals[i]);
//
//	delete data;
//	// generar normales -> recorrido de tri�ngulos
//	return m;
//}
