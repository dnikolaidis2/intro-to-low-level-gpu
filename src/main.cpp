#include <auto_vk_toolkit.hpp>
#include <imgui.h>

class triangle : public avk::invokee
{
public: // v== avk::invokee overrides which will be invoked by the framework ==v
	triangle(avk::queue& aQueue)
	: mQueue{ &aQueue }
	{}

	void initialize() override
	{
		std::vector<glm::vec3> triangleVertices {
			glm::vec3{ 0.0f, -0.5f, 1.0f},
			glm::vec3{ 0.5f,  0.5f, 1.0f},
			glm::vec3{-0.5f,  0.5f, 1.0f},
		};
		mVertexBuffer = avk::context().create_buffer(
			avk::memory_usage::device, {},
			avk::vertex_buffer_meta::create_from_data(triangleVertices)
		);

		auto vrtxFillCmd = mVertexBuffer->fill(triangleVertices.data(), 0);

		// Submit all the fill commands to the queue:
		auto fence = avk::context().record_and_submit_with_fence({
			 std::move(vrtxFillCmd),
		 }, *mQueue);
		// Wait on the host until the device is done:
		fence->wait_until_signalled();

		auto swapChainFormat = avk::context().main_window()->swap_chain_image_format();
		// Create our rasterization graphics pipeline with the required configuration:
		mPipeline = avk::context().create_graphics_pipeline_for(
			// Specify which shaders the pipeline consists of:
			avk::vertex_shader("shaders/basic.vert"),
			avk::fragment_shader("shaders/basic.frag"),
			// The next 3 lines define the format and location of the vertex shader inputs:
			// (The dummy values (like glm::vec3) tell the pipeline the format of the respective input)
			avk::from_buffer_binding(0) -> stream_per_vertex<glm::vec3>() -> to_location(0), // <-- corresponds to vertex shader's inPosition
			// Some further settings:
			avk::cfg::culling_mode::disabled,
			avk::cfg::viewport_depth_scissors_config::from_framebuffer(avk::context().main_window()->backbuffer_reference_at_index(0)),
			// We'll render to the back buffer, which has a color attachment always, and in our case additionally a depth
			// attachment, which has been configured when creating the window. See main() function!
			avk::context().create_renderpass({
				avk::attachment::declare(avk::format_from_window_color_buffer(avk::context().main_window()), avk::on_load::clear.from_previous_layout(avk::layout::undefined), avk::usage::color(0)     , avk::on_store::store),
			}, avk::context().main_window()->renderpass_reference().subpass_dependencies())
			);

		// set up updater
		// we want to use an updater, so create one:
		mUpdater.emplace();
		mUpdater->on(
			avk::swapchain_resized_event(avk::context().main_window()),
			avk::shader_files_changed_event(mPipeline.as_reference())
		).update(mPipeline);

		auto imguiManager = avk::current_composition()->element_by_type<avk::imgui_manager>();
		if(nullptr != imguiManager) {
			imguiManager->add_callback([this](){
				/*
				bool isEnabled = this->is_enabled();
				ImGui::Begin("Info & Settings");
				ImGui::SetWindowPos(ImVec2(1.0f, 1.0f), ImGuiCond_FirstUseEver);
				ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
				ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
				ImGui::TextColored(ImVec4(0.f, .6f, .8f, 1.f), "[F1]: Toggle input-mode");
				ImGui::TextColored(ImVec4(0.f, .6f, .8f, 1.f), " (UI vs. scene navigation)");
				ImGui::DragFloat3("Scale", glm::value_ptr(mScale), 0.005f, 0.01f, 10.0f);
				ImGui::Checkbox("Enable/Disable invokee", &isEnabled);
				if (isEnabled != this->is_enabled())
				{
					if (!isEnabled) this->disable();
					else this->enable();
				}

				mSrgbFrameBufferCheckbox->invokeImGui();
				mResizableWindowCheckbox->invokeImGui();
				mAdditionalAttachmentsCheckbox->invokeImGui();
				mNumConcurrentFramesSlider->invokeImGui();
				mNumPresentableImagesSlider->invokeImGui();
				mPresentationModeCombo->invokeImGui();

				ImGui::End();
				*/
			});
		}
	}

	void render() override
	{
		auto mainWnd = avk::context().main_window();

		// Get a command pool to allocate command buffers from:
		auto& commandPool = avk::context().get_command_pool_for_single_use_command_buffers(*mQueue);

		// The swap chain provides us with an "image available semaphore" for the current frame.
		// Only after the swapchain image has become available, we may start rendering into it.
		auto imageAvailableSemaphore = mainWnd->consume_current_image_available_semaphore();

		// Create a command buffer and render into the *current* swap chain image:
		auto cmdBfr = commandPool->alloc_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		avk::context().record({
			avk::command::render_pass(mPipeline->renderpass_reference(), avk::context().main_window()->current_backbuffer_reference(), {
				avk::command::bind_pipeline(mPipeline.as_reference()),
				avk::command::draw_vertices(3, 1, 0, 0, mVertexBuffer.as_reference()),
				})
		})
		.into_command_buffer(cmdBfr)
		.then_submit_to(*mQueue)
		// Do not start to render before the image has become available:
		.waiting_for(imageAvailableSemaphore >> avk::stage::color_attachment_output)
		.submit();

		mainWnd->handle_lifetime(std::move(cmdBfr));
	}

	void update() override
	{
	}

private: // v== Member variables ==v
	avk::queue* mQueue;
	avk::graphics_pipeline mPipeline;
	avk::buffer mVertexBuffer;
}; // triangle

int main() // <== Starting point ==
{
	int result = EXIT_FAILURE;
	try {
		// Create a window and open it
		auto mainWnd = avk::context().create_window("Model Loader");

		mainWnd->set_resolution({ 1000, 480 });
		mainWnd->enable_resizing(true);
		mainWnd->set_presentaton_mode(avk::presentation_mode::mailbox);
		mainWnd->set_number_of_concurrent_frames(3u);
		mainWnd->open();

		auto& singleQueue = avk::context().create_queue({}, avk::queue_selection_preference::versatile_queue, mainWnd);
		mainWnd->set_queue_family_ownership(singleQueue.family_index());
		mainWnd->set_present_queue(singleQueue);

		// Create an instance of our main avk::element which contains all the functionality:
		auto app = triangle(singleQueue);
		// Create another element for drawing the UI with ImGui
		auto ui = avk::imgui_manager(singleQueue);

		// Compile all the configuration parameters and the invokees into a "composition":
		auto composition = configure_and_compose(
			avk::application_name("Intro to Low-Level GPU"),
			[](avk::validation_layers& config) {
				config.enable_feature(vk::ValidationFeatureEnableEXT::eSynchronizationValidation);
			},
			// Pass windows:
			mainWnd,
			// Pass invokees:
			app, ui
			);

		// Create an invoker object, which defines the way how invokees/elements are invoked
		// (In this case, just sequentially in their execution order):
		avk::sequential_invoker invoker;

		// With everything configured, let us start our render loop:
		composition.start_render_loop(
			// Callback in the case of update:
			[&invoker](const std::vector<avk::invokee*>& aToBeInvoked) {
				// Call all the update() callbacks:
				invoker.invoke_updates(aToBeInvoked);
			},
			// Callback in the case of render:
			[&invoker](const std::vector<avk::invokee*>& aToBeInvoked) {
				// Sync (wait for fences and so) per window BEFORE executing render callbacks
				avk::context().execute_for_each_window([](avk::window* wnd) {
					wnd->sync_before_render();
				});

				// Call all the render() callbacks:
				invoker.invoke_renders(aToBeInvoked);

				// Render per window:
				avk::context().execute_for_each_window([](avk::window* wnd) {
					wnd->render_frame();
				});
			}
		); // This is a blocking call, which loops until avk::current_composition()->stop(); has been called (see update())

		result = EXIT_SUCCESS;
	}
	catch (avk::logic_error&) {}
	catch (avk::runtime_error&) {}
	return result;
}