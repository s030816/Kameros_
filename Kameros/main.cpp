#include "common.h"
#include "Shader.h"
#include "Renderer.h"

#include <shlobj.h>
#pragma comment(lib, "comctl32.lib")


GLfloat cam_x = 0.0f;
GLfloat cam_pitch = 45.0f;
GLfloat cam_yaw = 0.0f;
GLfloat cam_z = -20.0f;
GLfloat cam_y = 15.0f;
int cam_scale = 1;

double xpos, ypos;
double yoffset;
bool press1 = false;
bool press2 = false;

typedef CAMERA_DATA(*get_file_data_)(const wchar_t* path);
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void processInput(GLFWwindow *window);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
static std::vector<std::wstring> get_paths();
static std::vector<CAMERA_DATA> load_files();

int main()
{
    auto cams = load_files();
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Kameros", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    Renderer *rd = new Renderer();
    rd->add_cameras(cams);
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {

        processInput(window);

        rd->render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    
    delete rd;

    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
        ++cam_scale;
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        if(cam_scale > 1)
            --cam_scale;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    switch(button)
    {
        case GLFW_MOUSE_BUTTON_RIGHT:
            if(action == GLFW_PRESS) press1 = true;
            else press1 = false;
        break;
        case GLFW_MOUSE_BUTTON_LEFT:
            if(action == GLFW_PRESS) press2 = true;
            else press2 = false;
        break;
            
    }
    
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    
    if(press1)
    {
        cam_pitch -= static_cast<GLfloat>(ypos - ::ypos);
        cam_yaw -= static_cast<GLfloat>(xpos - ::xpos);
    }
    if(press2)
    {
        cam_x -= static_cast<GLfloat>(xpos - ::xpos);
        cam_y += static_cast<GLfloat>(ypos - ::ypos);
    }
    ::xpos = xpos;
    ::ypos = ypos;
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cam_z += static_cast<GLfloat>(yoffset);
    ::yoffset = yoffset;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
* Windows shell funkcija failu pasirinkimui
* @return Masyvas pasirinktu failu nuorodu
*/
std::vector<std::wstring> get_paths()
{
    IFileOpenDialog* pfd = NULL;
    COMDLG_FILTERSPEC FileTypes[] = { { L"JPG", L"*.JPG" } };
    std::vector<std::wstring> paths;
    HRESULT hResult = CoInitializeEx(NULL,
        COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pfd));

    if (!SUCCEEDED(hr))
    {
        return {};
    }
    DWORD dwOptions;
    hr = pfd->GetOptions(&dwOptions);

    if (SUCCEEDED(hr))
    {
        hr = pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
        hr = pfd->SetFileTypes(ARRAYSIZE(FileTypes), FileTypes);
    }
    else
    {
        return {};
    }

    if (SUCCEEDED(hr))
    {
        // Show the Open dialog.
        hr = pfd->Show(NULL);

        if (SUCCEEDED(hr))
        {
            // Obtain the result of the user interaction.
            IShellItemArray* psiaResults;
            hr = pfd->GetResults(&psiaResults);

            if (SUCCEEDED(hr))
            {
                DWORD count = 0;
                psiaResults->GetCount(&count);
                for (DWORD i = 0; i < count; ++i)
                {
                    IShellItem* item = nullptr;
                    wchar_t* pPath = nullptr;
                    psiaResults->GetItemAt(i, &item);
                    item->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
                    if (pPath)
                        paths.emplace_back(std::wstring(pPath));
                }
                psiaResults->Release();
            }
        }
    }
    else
    {
        return {};
    }
    pfd->Release();

    return paths;
}
/**
* Pagalbine funkcija konvertuoti gps koordinates i pasirinkta range
* @Param cams Kameru masyvas
* @Param range Virsutine range verte nuo 0; Pvz range = 10 -> 0 .. 10
*/
void normalize_gps(std::vector<CAMERA_DATA>& cams, double range)
{
    auto get_minmax = [&](int index)
    {
        double max = cams[0].gps[index];
        double min = max;
        for (const auto& i : cams)
        {
            if (i.gps[index] > max) max = i.gps[index];
            if (i.gps[index] < min) min = i.gps[index];
        }
        return std::make_pair(max-min, min);
    };

    auto minmax0 = get_minmax(0);
    auto minmax1 = get_minmax(1);
    auto minmax2 = get_minmax(2);

    auto new_value = [&](double val, double oldRange)
    {
        if (oldRange > 0.0) return val * range / oldRange;
        return oldRange;
    };

    for (auto& cam : cams)
    {
        cam.gps[0] -= minmax0.second;
        cam.gps[0] = new_value(cam.gps[0], minmax0.first);
        cam.gps[1] -= minmax1.second;
        cam.gps[1] = new_value(cam.gps[1], minmax1.first);
        cam.gps[2] -= minmax2.second;
        cam.gps[2] = new_value(cam.gps[2], minmax2.first);
    }

}



/**
* Dinamiskai prijungia biblioteka ir naudoja jos eksportuota funkcija
* duomenu is failo isgavimui
* @return Kameros duomenu masyvas (pozicija ir rotacija)
*/
std::vector<CAMERA_DATA> load_files()
{
    HMODULE hModule = LoadLibraryA("Exifparser64.dll");
    if (hModule)
    {
        auto get_file_data_f = (get_file_data_)GetProcAddress(hModule,
            "get_file_data");
        std::vector<CAMERA_DATA> cams;
        for (auto i : get_paths())
        {
            cams.emplace_back(get_file_data_f(i.c_str()));
        }
        FreeLibrary(hModule);
        if (cams.empty()) 
            return {};
        normalize_gps(cams, static_cast<double>(SCALE_RANGE));
        return cams;
    }
    else
    {
        std::cerr << "Cannot open dll: " << std::hex << GetLastError() << '\n';
    }
    return {};
}