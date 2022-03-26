// Forward declarations
class Scene;
class Camera;
struct GLFWwindow;

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;

class SceneRunner
{
public:
    SceneRunner();
    ~SceneRunner() = default;

    bool init(const std::string& title, int width = WINDOW_WIDTH, int height = WINDOW_HEIGHT, int samples = 0);
    int run(Camera* camera, Scene* scene);
    void loop();

    void OnPressKey(int key, int scancode, int action, int mods);
    void OnMouseMove(double x, double y);
    void OnMouseScroll(double xoffset, double yoffset);
    void OnResizeFramebuffer(int width, int height);

private:
    Camera* _camera = nullptr;
    Scene* _scene = nullptr;
    GLFWwindow* _window = nullptr;
    int _width = 0, _height = 0;
    int _fbw = 0, _fbh = 0;
    int _samples = 0;
    bool _debug = false; // Set true to enable debug messages

    float _deltaTime = 0.0f;
    float _lastFrame = 0.0f;
};
