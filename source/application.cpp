#include "application.hpp"
#include "tetrahedron.hpp"

#include <imgui.h>
#include <algorithm>
#include <array>
#include <cmath>

namespace {
const geometry::Mesh mesh = geometry::truncatedTetrahedron();
geometry::Vec3 angles{0.35f, 0.55f, 0.0f};
float zoom = 1.0f;
bool perspective = true;
bool wireframe = false;
bool auto_rotate = false;
constexpr float camera_distance = 6.0f;
}

namespace application {

bool initialize() {
	return true;
}

void shutdown() {
	auto& context = graphics::internal::context;
	vkQueueWaitIdle(context.graphics_queue);
}

void update([[maybe_unused]] double time) {
	if (auto_rotate) angles.y += ImGui::GetIO().DeltaTime * 0.45f;
	const auto* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::Begin("Truncated tetrahedron", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings);
	ImGui::TextUnformatted("Truncated regular tetrahedron: 12 vertices, 18 edges, 8 faces");
	ImGui::TextUnformatted("4 equilateral triangles + 4 regular hexagons | truncation: 1/3");
	ImGui::Checkbox("Perspective projection", &perspective);
	ImGui::SameLine();
	ImGui::Checkbox("Wireframe (all edges)", &wireframe);
	ImGui::SameLine();
	ImGui::Checkbox("Auto rotate", &auto_rotate);
	ImGui::SliderAngle("Rotation X", &angles.x);
	ImGui::SliderAngle("Rotation Y", &angles.y);
	ImGui::SliderAngle("Rotation Z", &angles.z);
	ImGui::SliderFloat("Zoom", &zoom, 0.3f, 2.0f);
	if (ImGui::Button("Reset view")) {
		angles = {0.35f, 0.55f, 0.0f};
		zoom = 1.0f;
		auto_rotate = false;
	}
	ImGui::SameLine();
	ImGui::TextUnformatted("Drag in the view to rotate. Scroll to zoom.");

	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const ImVec2 available = ImGui::GetContentRegionAvail();
	const ImVec2 size{std::max(available.x, 1.0f), std::max(available.y, 1.0f)};
	ImGui::InvisibleButton("Projection canvas", size);
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		angles.y += ImGui::GetIO().MouseDelta.x * 0.01f;
		angles.x += ImGui::GetIO().MouseDelta.y * 0.01f;
	}
	if (ImGui::IsItemHovered())
		zoom = std::clamp(zoom + ImGui::GetIO().MouseWheel * 0.1f, 0.3f, 2.0f);

	auto* draw = ImGui::GetWindowDrawList();
	const ImVec2 end{origin.x + size.x, origin.y + size.y};
	draw->PushClipRect(origin, end, true);
	draw->AddRectFilled(origin, end, IM_COL32(20, 25, 35, 255));
	const ImVec2 center{origin.x + size.x * 0.5f, origin.y + size.y * 0.5f};
	draw->AddLine({origin.x, center.y}, {end.x, center.y}, IM_COL32(45, 53, 67, 255));
	draw->AddLine({center.x, origin.y}, {center.x, end.y}, IM_COL32(45, 53, 67, 255));
	const float scale = std::min(size.x, size.y) * 0.30f * zoom;
	std::array<geometry::Vec3, 12> transformed;
	std::array<ImVec2, 12> projected;
	for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
		const auto p = geometry::rotate(mesh.vertices[i], angles);
		transformed[i] = p;
		// Camera at (0,0,d), looking toward the origin. The image plane is z=0.
		const float factor = perspective ? camera_distance / (camera_distance - p.z) : 1.0f;
		projected[i] = {center.x + scale * p.x * factor, center.y - scale * p.y * factor};
	}

	if (wireframe) {
		for (const auto& edge : mesh.edges)
			draw->AddLine(projected[edge[0]], projected[edge[1]], IM_COL32(180, 220, 255, 255), 2.0f);
	} else {
		struct VisibleFace { std::size_t index; float depth; geometry::Vec3 normal; };
		std::vector<VisibleFace> visible;
		for (std::size_t i = 0; i < mesh.faces.size(); ++i) {
			const auto& face = mesh.faces[i];
			geometry::Vec3 face_center{};
			for (int index : face.vertices) face_center = face_center + transformed[index];
			face_center = face_center * (1.0f / static_cast<float>(face.vertices.size()));
			const auto normal = geometry::rotate(face.normal, angles);
			const auto view = perspective ? geometry::Vec3{0, 0, camera_distance} - face_center
				: geometry::Vec3{0, 0, 1};
			if (geometry::dot(normal, view) > 0.0f) visible.push_back({i, face_center.z, normal});
		}
		std::sort(visible.begin(), visible.end(), [](const auto& a, const auto& b) {
			return a.depth < b.depth;
		});
		const auto light = geometry::normalized({-0.4f, 0.7f, 1.0f});
		for (const auto& item : visible) {
			const auto& face = mesh.faces[item.index];
			std::array<ImVec2, 6> polygon;
			for (std::size_t i = 0; i < face.vertices.size(); ++i) polygon[i] = projected[face.vertices[i]];
			const float shade = 0.35f + 0.65f * std::max(0.0f, geometry::dot(item.normal, light));
			const bool triangle = face.vertices.size() == 3;
			const ImU32 color = IM_COL32(static_cast<int>((triangle ? 245 : 75) * shade),
				static_cast<int>((triangle ? 170 : 170) * shade),
				static_cast<int>((triangle ? 75 : 235) * shade), 255);
			const int count = static_cast<int>(face.vertices.size());
			draw->AddConvexPolyFilled(polygon.data(), count, color);
			draw->AddPolyline(polygon.data(), count, IM_COL32(15, 20, 30, 255), ImDrawFlags_Closed, 2.0f);
		}
	}
	draw->PopClipRect();
	ImGui::End();
}

void render(const graphics::internal::FrameData& fd) {
	const auto& context = graphics::internal::context;
	vkResetCommandBuffer(fd.command_buffer, 0);
	const VkCommandBufferBeginInfo begin = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(fd.command_buffer, &begin);

	const VkClearValue clear_values[] = {
		{ .color = { { 0.08f, 0.08f, 0.12f, 1.0f } } },
		{ .depthStencil = { 1.0f, 0 } },
	};
	const VkRenderPassBeginInfo render_pass = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = context.render_pass,
		.framebuffer = fd.framebuffer,
		.renderArea = { .extent = context.swapchain_extent },
		.clearValueCount = 2,
		.pClearValues = clear_values,
	};
	vkCmdBeginRenderPass(fd.command_buffer, &render_pass, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdEndRenderPass(fd.command_buffer);
	vkEndCommandBuffer(fd.command_buffer);
}

} // namespace application
