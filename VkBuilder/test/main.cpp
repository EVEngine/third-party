#include "window.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include "glm/glm.hpp"
#define VKB_IMPL
#include "vkbuilder.hpp"

struct Vertex {
  glm::vec2 pos;
  glm::vec3 color;
  
  static vk::VertexInputBindingDescription 
  getBindingDescription(uint32_t binding) {
    vk::VertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = binding;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;
    return bindingDescription;
  }
  static std::vector<vk::VertexInputAttributeDescription>
  getAttributeDescription(uint32_t binding) {
    std::vector<vk::VertexInputAttributeDescription> attrs = {
      vk::VertexInputAttributeDescription(0, binding, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)),
      vk::VertexInputAttributeDescription(1, binding, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))
    };
    return attrs;
  }
};

extern void *create_surface_glfw(void *instance, void *window);

std::vector<uint32_t> readFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
  }
  size_t file_size = (size_t)file.tellg();
  std::vector<uint32_t> buffer((file_size + 3) / 4);
  file.seekg(0);
  file.read((char*)buffer.data(), static_cast<std::streamsize>(file_size));
  file.close();
  return buffer;
}

class Render {
public:
  void init(void *window) {
    vkb::InstanceBuilder builder;
    inst = builder.require_api_version(1, 2)
               .request_validation_layers()   // validate correctness
               .use_default_debug_messenger() // use DebugUtilsMessage
               .build();

    auto surface = (VkSurfaceKHR)create_surface_glfw(inst.instance, window);

    vkb::PhysicalDeviceSelector selector{inst};

    auto phys = selector.set_surface(surface)
            .set_minimum_version(1, 2) // require a vulkan 1.2 capable device
            .require_dedicated_transfer_queue()
            .select();

    vkb::DeviceBuilder device_builder{phys};

    // automatically propagate needed data from instance & physical device
    device = device_builder.build();

    create_swapchain();
    create_pipeline();
  }

  void create_swapchain() {
    vkb::SwapchainBuilder swapchain_builder{device};
    auto swap_ret = swapchain_builder.set_old_swapchain(swapchain).build();
    swapchain.destroy();
    swapchain = swap_ret;
  }

  void create_pipeline() {
    vkb::RenderPassBuilder renderpass_builder{device};
    renderpass = renderpass_builder
            .addPresentAttachment(swapchain.image_format, vk::AttachmentLoadOp::eClear)
            .addSubpass(vkb::SubpassBuilder()
              .addAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal))
            .addDependency(VK_SUBPASS_EXTERNAL, 0)
            .build();

    auto vert_code = readFile("vert.spv");
    auto frag_code = readFile("frag.spv");

    vkb::PipelineBuilder pipeline_builder{device, swapchain};
    pipeline = pipeline_builder
        .useClassicPipeline(vert_code, frag_code)
        .setVertexInputState(
          vkb::VertexInputStateBuilder()
            .addInputBinding<Vertex>()
            .addAttributeDescription<Vertex>()
        )
        .build(renderpass);

    vkb::PresentBuilder present_builder{device, swapchain};
    present = present_builder.build(renderpass);

    initVertex();
  }

  void initVertex() {
    v.resize(3);
    v[0].pos = {0, 0.5}; v[0].color = {1,0,0};
    v[1].pos = {-0.5, -0.5}; v[1].color = {0,1,0};
    v[2].pos = {0.5, -0.5}; v[2].color = {0,0,1};

    // create Vertex buffer
    buffer.allocate<Vertex>(device, v);
  }

  void render() {
    present.begin();
    present.beginRenderPass(renderpass);

    vk::DeviceSize offset(0);
    auto& cb = present.getCurrentCommandBuffer();
    cb.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    cb.bindVertexBuffers(0, 1, &buffer.buffer, &offset);
    cb.draw(3, 1, 0, 0);

    present.endRenderPass();
    present.end();
    present.drawFrame();
  }

  vkb::Instance inst;
  vkb::Device device;
  vkb::Swapchain swapchain;

  vk::RenderPass renderpass;
  vk::Pipeline pipeline;

  vkb::Present present;

  std::vector<Vertex> v;
  vkb::HostVertexBuffer buffer;
};

Render render;

void onRender() {
  render.render();
}

void initVulkan(void *window) {
  render.init(window);
}

int main() {
  GLFW_Window window;
  window.createWindow();
  initVulkan(window.getWindow());
  window.mainLoop();
  return 0;
}