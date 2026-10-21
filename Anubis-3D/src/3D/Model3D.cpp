#include "3D/Model3D.hpp"
#include "3D/Scene.hpp"
#include "Time.hpp"
#include "Cursor.hpp"
#include "BinaryReader.hpp"
#include "DebugLog.hpp"
#include "JsonReader.hpp"


namespace Anubis {

	int Model3D::_nextID = 0;

	Model3D::Model3D(float x, float y, float z, Quaternion rotation, float scale) {

		_id = _nextID++;

		_x = x;
		_y = y;
		_z = z;

		_rotation = rotation;

		_scale = scale;

		_meshes.clear();

		offset = sf::Vector2f(0.0f, 0.0f);
		_lastRotationMouse = sf::Vector2f(Anubis::cursor->_position);
	}

	Model3D::~Model3D() {

	}

	void Model3D::addMesh(std::shared_ptr<Mesh3D> mesh) {
		_meshes.push_back(mesh);
	}

	void Model3D::handleEvent(const sf::Event& event) {

		auto size = Anubis::render_window->getSize();
		auto mouse = Anubis::cursor->_position;

		if (size.x == 0 || size.y == 0)
			return;

		auto camera = scene->_camera;

		float sinAngle = std::sin(scene->_cameraAngle);
		float cosAngle = std::cos(scene->_cameraAngle);

		float relativeX = _x - camera.x;
		float relativeY = _y - camera.y;
		float relativeZ = _z - camera.z;

		float modelViewX = relativeX * cosAngle - relativeZ * sinAngle;
		float modelViewY = relativeY;

		float distance = -relativeX * sinAngle - relativeZ * cosAngle;
		float aspect = float(size.x) / float(size.y);

		float halfHeight = distance * std::tan(30.f * 3.14159265f / 180.f);
		float halfWidth = halfHeight * aspect;

		float mouseX = (2.f * mouse.x / size.x - 1.f) * halfWidth;
		float mouseY = (1.f - 2.f * mouse.y / size.y) * halfHeight;

		auto hovered = std::find(scene->_hovered.begin(), scene->_hovered.end(), this->shared_from_this());
		auto pressed = std::find(scene->_pressed.begin(), scene->_pressed.end(), this->shared_from_this());

		if (hovered != scene->_hovered.end()) {
			if (const auto* mbp = event.getIf<sf::Event::MouseButtonPressed>(); mbp && mbp->button == sf::Mouse::Button::Left) {

				if (std::find(
					scene->_pressed.begin(), scene->_pressed.end(), this->shared_from_this()
				) == scene->_pressed.end()) {
					scene->_pressed.push_back(this->shared_from_this());
				}
			}
		}

		pressed = std::find(scene->_pressed.begin(), scene->_pressed.end(), this->shared_from_this());

		if (pressed != scene->_pressed.end()) {
			if (const auto* mbp = event.getIf<sf::Event::MouseButtonPressed>(); mbp) {
				if (mbp->button == sf::Mouse::Button::Left) {
					offset.x = mouseX - modelViewX;
					offset.y = mouseY - modelViewY;
				}

				if (mbp->button == sf::Mouse::Button::Right) {
					_lastRotationMouse = sf::Vector2f(mbp->position);
				}
			}
		}

		if (pressed != scene->_pressed.end()) {
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && cursor->_isDragging) {
				float viewX = mouseX - offset.x;
				float viewY = mouseY - offset.y;

				_x = camera.x + viewX * cosAngle - distance * sinAngle;
				_y = camera.y + viewY;
				_z = camera.z - viewX * sinAngle - distance * cosAngle;
			}
		}
	}

	void Model3D::update() {

		auto pressed = std::find(scene->_pressed.begin(), scene->_pressed.end(), this->shared_from_this());

		if (pressed != scene->_pressed.end() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
		{
			auto delta = sf::Vector2f(Anubis::cursor->_position) - _lastRotationMouse;
			_lastRotationMouse = sf::Vector2f(Anubis::cursor->_position);

			float dx = (float)(delta.x);
			float dy = (float)(delta.y);
			float length = std::hypot(dx, dy);
			if (length > 0.f) {
				float sensitivity = 0.5f; // Stopnie na piksel.
				float toRadians = 3.14159265f / 180.f;

				float halfAngle = length * sensitivity * toRadians * 0.5f;
				float s = std::sin(halfAngle) / length;

				float sinAngle = std::sin(scene->_cameraAngle);
				float cosAngle = std::cos(scene->_cameraAngle);

				Quaternion deltaRotation{
					std::cos(halfAngle),
					dy * s * cosAngle,
					dx * s,
					-dy * s * sinAngle
				};

				_rotation = deltaRotation * _rotation;
				_rotation.normalize();
			}

			return;
		}

		// rotation about Y axis
		float speed = 30.f; // Stopnie na sekundę.
		float toRadians = 3.14159265f / 180.f;
		float angle = speed * Anubis::deltaTime.asSeconds() * toRadians;

		Quaternion deltaRotation{
			std::cos(angle * 0.5f), // w
			0,						// x
			std::sin(angle * 0.5f), // y
			0.f						// z      
		};

		_rotation = deltaRotation * _rotation;
		_rotation.normalize();
	}

	void Model3D::drawShape() {
		for (auto& mesh : _meshes) {
			mesh->drawShape(_x, _y, _z, _rotation, _scale);
		}
	}

	void Model3D::draw() {

		auto hovered = std::find(scene->_hovered.begin(), scene->_hovered.end(), this->shared_from_this());
		auto pressed = std::find(scene->_pressed.begin(), scene->_pressed.end(), this->shared_from_this());

		for (auto& mesh : _meshes) {
			bool highlighting = pressed != scene->_pressed.end() || (scene->_pressed.empty() && hovered != scene->_hovered.end());
			mesh->draw(highlighting, _x, _y, _z, _rotation, _scale);
		}
	}

	std::shared_ptr<Model3D> openModel3DFromFile(std::filesystem::path path) {

		DebugLog(L"Load the model \"" + path.wstring() + L"\"");

		std::ifstream file;
		file.open(path, std::ios::in | std::ios::binary);
		if (!file) {
			DebugError(L"Nie udało się otworzyć modelu");
			return nullptr;
		}
		
		// .. loading ..

		BinaryReader reader(file);

		uint32_t magic = reader.read_uint32();
		uint32_t version = reader.read_uint32();
		uint32_t length = reader.read_uint32();

		if (magic != 0x46546C67) {
			DebugError(L"Plik nie jest plikiem GLB");
			return nullptr;
		}

		if (version != 2) {
			DebugError(L"Nieobslugiwana wersja GLB");
			return nullptr;
		}

		DebugStat(L"Wersja: " + std::to_wstring(version));
		DebugStat(L"Rozmiar: " + std::to_wstring(length) + L" bajtow");

		uint32_t chunkLength = reader.read_uint32();
		uint32_t chunkType = reader.read_uint32();

		if (chunkType != 0x4E4F534A) { // "JSON"
			DebugError(L"Pierwsza część GLB nie jest JSON-em");
			return nullptr;
		}

		if (length < 20 || chunkLength > length - 20) {
			DebugError(L"Nieprawidłowy rozmiar części JSON");
			return nullptr;
		}

		std::string jsonText(chunkLength, '\0');

		if (!file.read(jsonText.data(), chunkLength)) {
			DebugError(L"Nie udało się odczytać części JSON");
			return nullptr;
		}

		DebugStat(L"Rozmiar JSON: " + std::to_wstring(chunkLength));

		uint32_t binLength = reader.read_uint32();
		uint32_t binType = reader.read_uint32();

		if (binType != 0x004E4942) { // "BIN"
			DebugError(L"Oczekiwano części BIN");
			return nullptr;
		}

		// Nagłówek GLB + nagłówek JSON + JSON + nagłówek BIN.
		const uint64_t binStart = 12ull + 8ull + chunkLength + 8ull;

		if (binStart > length || binLength > length - binStart) {
			DebugError(L"Nieprawidłowy rozmiar części BIN");
			return nullptr;
		}

		std::vector<uint8_t> binData(binLength);

		if (!file.read(reinterpret_cast<char*>(binData.data()), binLength)) {
			DebugError(L"Nie udało się odczytać części BIN");
			return nullptr;
		}

		DebugStat(L"Rozmiar BIN: " + std::to_wstring(binLength));
		
		// Json reading

		JsonReader jsonReader(jsonText);

		if (!jsonReader.findRootField("meshes"))
			return nullptr;

		if (!jsonReader.expect('['))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("primitives"))
			return nullptr;

		if (!jsonReader.expect('['))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("attributes"))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("POSITION"))
			return nullptr;

		int positionAccessor = 0;

		if (!jsonReader.readInt(positionAccessor))
			return nullptr;

		DebugStat(L"Accessor position: " + std::to_wstring(positionAccessor));

		if (!jsonReader.findRootField("accessors"))
			return nullptr;

		if (!jsonReader.enterArrayElement(positionAccessor))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int positionAccessorStart = jsonReader._position;

		if (!jsonReader.findField("count"))
			return nullptr;

		int vertexCount = 0;

		if (!jsonReader.readInt(vertexCount))
			return nullptr;

		DebugStat(L"Vertices count: " + std::to_wstring(vertexCount));
		
		jsonReader._position = positionAccessorStart;

		if (!jsonReader.findField("bufferView"))
			return nullptr;

		int positionBufferView = 0;

		if (!jsonReader.readInt(positionBufferView))
			return nullptr;

		DebugStat(L"BufferView position: " + std::to_wstring(positionBufferView));

		if (!jsonReader.findRootField("bufferViews"))
			return nullptr;

		if (!jsonReader.enterArrayElement(positionBufferView))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int positionBufferViewStart = jsonReader._position;

		if (!jsonReader.findField("byteOffset"))
			return nullptr;

		int positionBufferOffset = 0;

		if (!jsonReader.readInt(positionBufferOffset))
			return nullptr;

		DebugStat(L"Position buffer offset: " + std::to_wstring(positionBufferOffset));

		jsonReader._position = positionAccessorStart;

		if (!jsonReader.findField("componentType"))
			return nullptr;

		int positionComponentType = 0;

		if (!jsonReader.readInt(positionComponentType))
			return nullptr;

		if (positionComponentType != 5126) { // float
			DebugError(L"Positions are not stored as floats");
			return nullptr;
		}

		jsonReader._position = positionAccessorStart;

		if (!jsonReader.findField("type"))
			return nullptr;

		std::string positionType = jsonReader.readString();

		if (positionType != "VEC3") {
			DebugError(L"Positions are not stored as VEC3");
			return nullptr;
		}

		DebugStat(L"Position format: float, VEC3");

		jsonReader._position = positionBufferViewStart;

		if (!jsonReader.findField("byteStride"))
			return nullptr;

		int positionStride = 0;

		if (!jsonReader.readInt(positionStride))
			return nullptr;

		if (positionStride < 12 || positionStride % 4 != 0) {
			DebugError(L"Invalid position byte stride");
			return nullptr;
		}

		DebugStat(L"Position byte stride: " + std::to_wstring(positionStride));

		jsonReader._position = positionAccessorStart;

		int positionAccessorOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(positionAccessorOffset))
				return nullptr;
		}

		DebugStat(L"Position accessor offset: " + std::to_wstring(positionAccessorOffset));

		// Load The Mesh
		const uint64_t positionsStart = static_cast<uint64_t>(positionBufferOffset) + static_cast<uint64_t>(positionAccessorOffset);

		if (vertexCount <= 0) {
			DebugError(L"Invalid vertex count");
			return nullptr;
		}

		const uint64_t positionsEnd = positionsStart + static_cast<uint64_t>(vertexCount - 1) * positionStride + 12;

		if (positionsEnd > binData.size()) {
			DebugError(L"Position data exceeds BIN bounds");
			return nullptr;
		}

		std::shared_ptr<Mesh3D> mesh = std::make_shared<Mesh3D>();
		mesh->_vertices.reserve(vertexCount);

		for (int i = 0; i < vertexCount; ++i) {
			const uint64_t offset = positionsStart + static_cast<uint64_t>(i) * positionStride;

			float position[3];

			std::memcpy(
				position,
				binData.data() + offset,
				sizeof(position)
			);

			mesh->_vertices.emplace_back(
				position[0], position[1], position[2],
				0.f, 0.f, 0.f, // normals — later
				0.f, 0.f       // UV — later
			);
		}

		DebugStat(L"Loaded vertices: " + std::to_wstring(mesh->_vertices.size()));

		if (!jsonReader.findRootField("meshes"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("primitives"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("indices"))
			return nullptr;

		int indicesAccessor = 0;

		if (!jsonReader.readInt(indicesAccessor))
			return nullptr;

		DebugStat(L"Indices accessor: " + std::to_wstring(indicesAccessor));

		if (!jsonReader.findRootField("accessors"))
			return nullptr;

		if (!jsonReader.enterArrayElement(indicesAccessor))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int indicesAccessorStart = jsonReader._position;

		if (!jsonReader.findField("count"))
			return nullptr;

		int indicesCount = 0;

		if (!jsonReader.readInt(indicesCount))
			return nullptr;

		DebugStat(L"Indices count: " + std::to_wstring(indicesCount));

		jsonReader._position = indicesAccessorStart;

		if (!jsonReader.findField("componentType"))
			return nullptr;

		int indicesComponentType = 0;

		if (!jsonReader.readInt(indicesComponentType))
			return nullptr;

		// Na tym etapie obsługujemy indeksy uint32_t.
		if (indicesComponentType != 5125) {
			DebugError(L"Only uint32 indices are currently supported");
			return nullptr;
		}

		jsonReader._position = indicesAccessorStart;

		if (!jsonReader.findField("type"))
			return nullptr;

		if (jsonReader.readString() != "SCALAR") {
			DebugError(L"Indices are not stored as SCALAR");
			return nullptr;
		}

		jsonReader._position = indicesAccessorStart;

		if (!jsonReader.findField("bufferView"))
			return nullptr;

		int indicesBufferView = 0;

		if (!jsonReader.readInt(indicesBufferView))
			return nullptr;

		DebugStat(L"Indices format: uint32, SCALAR");
		DebugStat(L"Indices buffer view: " + std::to_wstring(indicesBufferView));

		// Optional offset inside the buffer view.
		jsonReader._position = indicesAccessorStart;

		int indicesAccessorOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(indicesAccessorOffset))
				return nullptr;
		}

		// Find the buffer view.
		if (!jsonReader.findRootField("bufferViews"))
			return nullptr;

		if (!jsonReader.enterArrayElement(indicesBufferView))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int indicesBufferViewStart = jsonReader._position;

		int indicesBufferOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(indicesBufferOffset))
				return nullptr;
		}

		DebugStat(L"Indices accessor offset: " + std::to_wstring(indicesAccessorOffset));
		DebugStat(L"Indices buffer offset: " + std::to_wstring(indicesBufferOffset));

		const uint64_t indicesStart = static_cast<uint64_t>(indicesBufferOffset) + static_cast<uint64_t>(indicesAccessorOffset);

		if (indicesCount <= 0 || indicesCount % 3 != 0) {
			DebugError(L"Invalid index count for triangles");
			return nullptr;
		}

		const uint64_t indicesBytes = static_cast<uint64_t>(indicesCount) * sizeof(uint32_t);

		if (indicesStart > binData.size() ||
			indicesBytes > binData.size() - indicesStart) {
			DebugError(L"Index data exceeds BIN bounds");
			return nullptr;
		}

		mesh->_indices.resize(indicesCount);

		std::memcpy(
			mesh->_indices.data(),
			binData.data() + indicesStart,
			static_cast<std::size_t>(indicesBytes)
		);

		for (const auto index : mesh->_indices) {
			if (index >= mesh->_vertices.size()) {
				DebugError(L"Index exceeds vertex count");
				return nullptr;
			}
		}

		DebugStat(L"Loaded indices: " + std::to_wstring(mesh->_indices.size()));

		if (!jsonReader.findRootField("meshes"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("primitives"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("attributes"))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("NORMAL"))
			return nullptr;

		int normalAccessor = 0;

		if (!jsonReader.readInt(normalAccessor))
			return nullptr;

		DebugStat(L"Normal accessor: " + std::to_wstring(normalAccessor));

		if (!jsonReader.findRootField("accessors"))
			return nullptr;

		if (!jsonReader.enterArrayElement(normalAccessor))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int normalAccessorStart = jsonReader._position;

		if (!jsonReader.findField("count"))
			return nullptr;

		int normalCount = 0;

		if (!jsonReader.readInt(normalCount))
			return nullptr;

		if (normalCount != vertexCount) {
			DebugError(L"Normal count does not match vertex count");
			return nullptr;
		}

		jsonReader._position = normalAccessorStart;

		if (!jsonReader.findField("bufferView"))
			return nullptr;

		int normalBufferView = 0;

		if (!jsonReader.readInt(normalBufferView))
			return nullptr;

		DebugStat(L"Normal count: " + std::to_wstring(normalCount));
		DebugStat(L"Normal buffer view: " + std::to_wstring(normalBufferView));

		jsonReader._position = normalAccessorStart;

		if (!jsonReader.findField("componentType"))
			return nullptr;

		int normalComponentType = 0;

		if (!jsonReader.readInt(normalComponentType))
			return nullptr;

		if (normalComponentType != 5126) {
			DebugError(L"Normals are not stored as floats");
			return nullptr;
		}

		jsonReader._position = normalAccessorStart;

		if (!jsonReader.findField("type"))
			return nullptr;

		if (jsonReader.readString() != "VEC3") {
			DebugError(L"Normals are not stored as VEC3");
			return nullptr;
		}

		jsonReader._position = normalAccessorStart;

		int normalAccessorOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(normalAccessorOffset))
				return nullptr;
		}

		DebugStat(L"Normal format: float, VEC3");
		DebugStat(L"Normal accessor offset: " + std::to_wstring(normalAccessorOffset));

		if (!jsonReader.findRootField("bufferViews"))
			return nullptr;

		if (!jsonReader.enterArrayElement(normalBufferView))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int normalBufferViewStart = jsonReader._position;

		int normalBufferOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(normalBufferOffset))
				return nullptr;
		}

		jsonReader._position = normalBufferViewStart;

		int normalStride = 12; // Default for float, VEC3.

		if (jsonReader.findField("byteStride", false)) {
			if (!jsonReader.readInt(normalStride))
				return nullptr;
		}

		if (normalStride < 12 || normalStride > 252 || normalStride % 4 != 0) {
			DebugError(L"Invalid normal byte stride");
			return nullptr;
		}

		DebugStat(L"Normal buffer offset: " + std::to_wstring(normalBufferOffset));
		DebugStat(L"Normal byte stride: " + std::to_wstring(normalStride));

		const uint64_t normalsStart = static_cast<uint64_t>(normalBufferOffset) + static_cast<uint64_t>(normalAccessorOffset);

		if (normalCount <= 0 || static_cast<std::size_t>(normalCount) != mesh->_vertices.size()) {
			DebugError(L"Invalid normal count");
			return nullptr;
		}

		const uint64_t normalsEnd = normalsStart + static_cast<uint64_t>(normalCount - 1) * normalStride + 12;

		if (normalsEnd > binData.size()) {
			DebugError(L"Normal data exceeds BIN bounds");
			return nullptr;
		}

		for (int i = 0; i < normalCount; ++i) {
			const uint64_t offset = normalsStart + static_cast<uint64_t>(i) * normalStride;

			float normal[3];

			std::memcpy(
				normal,
				binData.data() + offset,
				sizeof(normal)
			);

			auto& vertex = mesh->_vertices[i];

			vertex._nx = normal[0];
			vertex._ny = normal[1];
			vertex._nz = normal[2];
		}

		DebugStat(L"Loaded normals: " + std::to_wstring(normalCount));

		if (!jsonReader.findRootField("meshes"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("primitives"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("attributes"))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("TEXCOORD_0"))
			return nullptr;

		int uvAccessor = 0;

		if (!jsonReader.readInt(uvAccessor))
			return nullptr;

		DebugStat(L"UV accessor: " + std::to_wstring(uvAccessor));

		if (!jsonReader.findRootField("accessors"))
			return nullptr;

		if (!jsonReader.enterArrayElement(uvAccessor))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int uvAccessorStart = jsonReader._position;

		if (!jsonReader.findField("count"))
			return nullptr;

		int uvCount = 0;

		if (!jsonReader.readInt(uvCount))
			return nullptr;

		if (uvCount != vertexCount) {
			DebugError(L"UV count does not match vertex count");
			return nullptr;
		}

		jsonReader._position = uvAccessorStart;

		if (!jsonReader.findField("bufferView"))
			return nullptr;

		int uvBufferView = 0;

		if (!jsonReader.readInt(uvBufferView))
			return nullptr;

		DebugStat(L"UV count: " + std::to_wstring(uvCount));
		DebugStat(L"UV buffer view: " + std::to_wstring(uvBufferView));

		jsonReader._position = uvAccessorStart;

		if (!jsonReader.findField("componentType"))
			return nullptr;

		int uvComponentType = 0;

		if (!jsonReader.readInt(uvComponentType))
			return nullptr;

		if (uvComponentType != 5126) {
			DebugError(L"Only float UV coordinates are currently supported");
			return nullptr;
		}

		jsonReader._position = uvAccessorStart;

		if (!jsonReader.findField("type"))
			return nullptr;

		if (jsonReader.readString() != "VEC2") {
			DebugError(L"UV coordinates are not stored as VEC2");
			return nullptr;
		}

		jsonReader._position = uvAccessorStart;

		int uvAccessorOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(uvAccessorOffset))
				return nullptr;
		}

		DebugStat(L"UV format: float, VEC2");
		DebugStat(L"UV accessor offset: " + std::to_wstring(uvAccessorOffset));

		if (!jsonReader.findRootField("bufferViews"))
			return nullptr;

		if (!jsonReader.enterArrayElement(uvBufferView))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int uvBufferViewStart = jsonReader._position;

		int uvBufferOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(uvBufferOffset))
				return nullptr;
		}

		jsonReader._position = uvBufferViewStart;

		int uvStride = 8; // Two floats: u, v.

		if (jsonReader.findField("byteStride", false)) {
			if (!jsonReader.readInt(uvStride))
				return nullptr;
		}

		if (uvStride < 8 || uvStride > 252 || uvStride % 4 != 0) {
			DebugError(L"Invalid UV byte stride");
			return nullptr;
		}

		DebugStat(L"UV buffer offset: " + std::to_wstring(uvBufferOffset));
		DebugStat(L"UV byte stride: " + std::to_wstring(uvStride));

		const uint64_t uvStart = static_cast<uint64_t>(uvBufferOffset) + static_cast<uint64_t>(uvAccessorOffset);

		if (uvCount <= 0 ||
			static_cast<std::size_t>(uvCount) != mesh->_vertices.size()) {
			DebugError(L"Invalid UV count");
			return nullptr;
		}

		const uint64_t uvEnd = uvStart + static_cast<uint64_t>(uvCount - 1) * uvStride + 8;

		if (uvEnd > binData.size()) {
			DebugError(L"UV data exceeds BIN bounds");
			return nullptr;
		}

		for (int i = 0; i < uvCount; ++i) {
			const uint64_t offset = uvStart + static_cast<uint64_t>(i) * uvStride;

			float uv[2];

			std::memcpy(
				uv,
				binData.data() + offset,
				sizeof(uv)
			);

			auto& vertex = mesh->_vertices[i];

			vertex._u = uv[0];
			vertex._v = uv[1];
		}

		DebugStat(L"Loaded UV coordinates: " + std::to_wstring(uvCount));

		if (!jsonReader.findRootField("meshes"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("primitives"))
			return nullptr;

		if (!jsonReader.enterArrayElement(0))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("material"))
			return nullptr;

		int materialIndex = 0;

		if (!jsonReader.readInt(materialIndex))
			return nullptr;

		DebugStat(L"Material index: " + std::to_wstring(materialIndex));

		if (!jsonReader.findRootField("materials"))
			return nullptr;

		if (!jsonReader.enterArrayElement(materialIndex))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("pbrMetallicRoughness"))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("baseColorTexture"))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("index"))
			return nullptr;

		int baseColorTextureIndex = 0;

		if (!jsonReader.readInt(baseColorTextureIndex))
			return nullptr;

		DebugStat(L"Base color texture index: " + std::to_wstring(baseColorTextureIndex));

		if (!jsonReader.findRootField("textures"))
			return nullptr;

		if (!jsonReader.enterArrayElement(baseColorTextureIndex))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		if (!jsonReader.findField("source"))
			return nullptr;

		int baseColorImageIndex = 0;

		if (!jsonReader.readInt(baseColorImageIndex))
			return nullptr;

		DebugStat(L"Base color image index: " + std::to_wstring(baseColorImageIndex));

		if (!jsonReader.findRootField("images"))
			return nullptr;

		if (!jsonReader.enterArrayElement(baseColorImageIndex))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int baseColorImageStart = jsonReader._position;

		if (!jsonReader.findField("mimeType"))
			return nullptr;

		if (jsonReader.readString() != "image/png") {
			DebugError(L"Only PNG images are currently supported");
			return nullptr;
		}

		jsonReader._position = baseColorImageStart;

		if (!jsonReader.findField("bufferView"))
			return nullptr;

		int baseColorImageBufferView = 0;

		if (!jsonReader.readInt(baseColorImageBufferView))
			return nullptr;

		DebugStat(L"Base color image format: PNG");
		DebugStat(L"Base color image buffer view: " + std::to_wstring(baseColorImageBufferView));

		if (!jsonReader.findRootField("bufferViews"))
			return nullptr;

		if (!jsonReader.enterArrayElement(baseColorImageBufferView))
			return nullptr;

		if (!jsonReader.expect('{'))
			return nullptr;

		int imageBufferViewStart = jsonReader._position;

		int imageBufferOffset = 0;

		if (jsonReader.findField("byteOffset", false)) {
			if (!jsonReader.readInt(imageBufferOffset))
				return nullptr;
		}

		jsonReader._position = imageBufferViewStart;

		if (!jsonReader.findField("byteLength"))
			return nullptr;

		int imageByteLength = 0;

		if (!jsonReader.readInt(imageByteLength))
			return nullptr;

		if (imageByteLength <= 0 ||
			static_cast<uint64_t>(imageBufferOffset) > binData.size() ||
			static_cast<uint64_t>(imageByteLength) >
			binData.size() - static_cast<uint64_t>(imageBufferOffset)) {
			DebugError(L"Image data exceeds BIN bounds");
			return nullptr;
		}

		DebugStat(L"Image buffer offset: " + std::to_wstring(imageBufferOffset));
		DebugStat(L"Image byte length: " + std::to_wstring(imageByteLength));

		auto image = std::make_shared<sf::Image>();

		if (!image->loadFromMemory(
			binData.data() + imageBufferOffset,
			static_cast<std::size_t>(imageByteLength))) {
			DebugError(L"Failed to decode base color image");
			return nullptr;
		}

		auto texture = std::make_shared<Texture>(
			path.wstring() + L"#baseColor",
			image
		);

		if (texture->_texture->getSize().x == 0 ||
			texture->_texture->getSize().y == 0) {
			DebugError(L"Failed to create base color texture");
			return nullptr;
		}

		mesh->setTexture(texture);

		DebugStat(L"Base color texture loaded");

		file.close();

		std::shared_ptr<Model3D> model = std::make_shared<Model3D>();
		mesh->uploadToGPU();
		model->addMesh(mesh);

		return model;

	}
}
