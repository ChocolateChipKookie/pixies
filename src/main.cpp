#include <iostream>
#include "Context.h"
#include "Shader.h"
#include <array>
#include <memory>
#include <utility>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <kki/random.h>
#include <chrono>
#include <thread>
#include <FastNoiseLite.h>
#include <cstring>

class Drawable{
    virtual void draw(std::shared_ptr<kki::Shader> shader) = 0;
};

class Texture : public Drawable{
private:
    unsigned _texture{0};
    unsigned _vao{}, _vbo{}, _ebo{};
    //                  Positions       Texture coords
    float vertices[16]={ -1.f, -1.f,     0.f, 0.f,
                          1.f, -1.f,     1.f, 0.f,
                          1.f,  1.f,     1.f, 1.f,
                         -1.f,  1.f,     0.f, 1.f};
    unsigned indeces[6] = {0, 1, 2, 0, 2, 3};

protected:
    std::vector<glm::vec3> texture_data;
    size_t width;
    size_t height;
    bool changed = false;
public:

    Texture(int width, int height)
        : texture_data(width * height),
          width(width),
          height(height)

    {
        // VAO and VBO
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeces), indeces, GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenTextures(1, &_texture);
        glBindTexture(GL_TEXTURE_2D, _texture);
        //Set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, texture_data.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        changed = false;
    }

    [[nodiscard]]
    size_t getWidth() const {
        return width;
    }

    [[nodiscard]]
    size_t getHeight() const {
        return height;
    }


    glm::vec3& operator()(size_t x, size_t y){
        return texture_data[y * width + x];
    }

    glm::vec3& at(size_t x, size_t y){
        return texture_data[y * width + x];
    }

    virtual void update() = 0;

    void updateTexture(){
        glBindTexture(GL_TEXTURE_2D, _texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, texture_data.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void draw(std::shared_ptr<kki::Shader> shader) override{
        shader->use();
        if (changed){
            updateTexture();
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _texture);
        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

class Pixies : public Drawable{
private:
    unsigned _vao{}, _vbo[2]{};
protected:
    std::vector<glm::vec2> positions;
    std::vector<glm::vec3> colours;
    size_t pixies;
public:

    Pixies(std::vector<glm::vec2>  vertices, std::vector<glm::vec3>  colours)
            : positions(std::move(vertices)),
              colours(std::move(colours)),
              pixies(positions.size())
    {
        assert(vertices.size() == colours.size());
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(2, _vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), positions.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(positions[0]), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(colours[0]), colours.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(colours[0]), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    explicit Pixies(size_t count, bool random_start = true)
            : pixies(count)
    {
        if (random_start){
            auto rng = kki::rng::global;
            positions.reserve(pixies);
            for (size_t i = 0; i < pixies; ++i){
                positions.emplace_back(rng.randomReal(-1.f, 1.f), rng.randomReal(-1.f, 1.f));
            }
            colours.resize(pixies, {1, 0, 0});
        } else {
            positions.resize(pixies);
            colours.resize(pixies, {1, 0, 0});
        }

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(2, _vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(positions[0]), positions.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(positions[0]), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(colours[0]), colours.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(colours[0]), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void draw(std::shared_ptr<kki::Shader> shader) override{
        shader->use();
        glBindVertexArray(_vao);
        glDrawArrays(GL_POINTS, 0, positions.size());

    }

    void updateBuffer(){
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(positions[0]), positions.data());
        glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, colours.size() * sizeof(colours[0]), colours.data());
    }

    virtual void update(size_t i) = 0;

    void updateAll(){
        for(size_t i = 0; i < pixies; ++i){
            update(i);
        }
        updateBuffer();
    }
};

class NoiseBackground : public Texture{
    FastNoiseLite noise;
    float current_z = 0.f;

public:
    NoiseBackground(int width, int height)
            :   Texture(width, height)
    {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(2);
    }

    void update() override{
        float x = 0.f;
        float x_inc = 2.f;
        float y = 0.f;
        float y_inc = 2.f;

        Texture& tex = *this;

        for (unsigned y_i = 0; y_i < height; y_i++){
            for (unsigned x_i = 0; x_i < width; x_i++){
                float val = noise.GetNoise(x, y, current_z);
                tex(x_i, y_i) = {val, val, val};
                x += x_inc;
            }
            y += y_inc;
            x = 0.f;
        }

        changed = true;
        current_z += 0.5;
    }

};

class NaivePixies : public Pixies{
private:
    kki::Random& rng = kki::rng::global;
    const float PI = 3.141592;
    const float noise_mapping = 4 * PI;

    struct pixieData{
        float speed = 0;
        float direction = 0;
    };

    std::vector<pixieData> _data;
    NoiseBackground& _background;
public:
    explicit NaivePixies(NoiseBackground& background, size_t count, bool random_start)
            :   Pixies(count, random_start), _background(background)
    {
        _data.reserve(count);
        for (unsigned i = 0; i < count; ++i){
            _data.push_back({0.01, rng.randomReal(0.f, 2 * PI)});
        }
    }

    void update(size_t i) override{
        auto& pos = positions[i];
        auto& col = colours[i];
        auto& mov = _data[i];

        // Update direction
        float val = _background(static_cast<size_t>((pos.x + 1.f) * 499.f / 2.f), static_cast<size_t>((pos.y + 1.f) * 499.f / 2.f))[0];
        mov.direction = val * noise_mapping;


        // Update position
        float d_x = glm::cos(mov.direction) * mov.speed;
        float d_y = glm::sin(mov.direction) * mov.speed;

        pos.x = pos.x + d_x;
        pos.y = pos.y + d_y;

        // If they are outside, reassign the position
        if ( pos.x < -0.99f || pos.y > 0.99f){
            pos = {rng.randomReal(-0.99f, 0.99f), rng.randomReal(-0.99f, 0.99f)};
        }

        // Update colour
        auto red = [](float x){return std::min(std::max(std::abs(6 * x - 3) - 1, 0.f), 0.99f);};
        auto green = [](float x){return std::min(std::max(-std::abs(6 * x - 2) + 2, 0.f), 0.99f);};
        auto blue = [](float x){return std::min(std::max(-std::abs(6 * x - 4) + 2, 0.f), 0.99f);};

        col = {red(val), green(val), blue(val)};
    }
};

class PlanktonPixies : public Pixies{
private:
    kki::Random& rng = kki::rng::global;
    const float PI = 3.141592;
    const float noise_mapping = 4 * PI;

    const float _max_speed = 0.01;
//    const float _acceleration = 0.0015;   // Perlin 3 octaves
    const float _acceleration = 0.0065;

    std::vector<glm::vec2> _speed;
    NoiseBackground& _background;
public:
    explicit PlanktonPixies(NoiseBackground& background, size_t count, bool random_start)
            :   Pixies(count, random_start), _background(background)
    {
        _speed.reserve(count);
        for (unsigned i = 0; i < count; ++i){
            _speed.emplace_back(0.f, 0.f);
        }
    }

    void update(size_t i) override{
        auto& pos = positions[i];
        auto& col = colours[i];
        auto& mov = _speed[i];

        // Update direction
        float val = _background(static_cast<size_t>((pos.x + 1.f) * 499.f / 2.f), static_cast<size_t>((pos.y + 1.f) * 499.f / 2.f))[0];
        float angle = val * noise_mapping;

        glm::vec2 d_v{glm::cos(angle), glm::sin(angle)};
        mov += d_v * _acceleration;
        if (glm::length(mov) > _max_speed){
            mov = glm::normalize(mov) * _max_speed;
        }

        // Update position
        pos += mov;

        // If they are outside, reassign the position
        if ( pos.x < -0.99f || pos.y > 0.99f){
            pos = {rng.randomReal(-0.99f, 0.99f), rng.randomReal(-0.99f, 0.99f)};
        }

        // Update colour
        auto red = [](float x){return std::min(std::max(std::abs(6 * x - 3) - 1, 0.f), 0.99f);};
        auto green = [](float x){return std::min(std::max(-std::abs(6 * x - 2) + 2, 0.f), 0.99f);};
        auto blue = [](float x){return std::min(std::max(-std::abs(6 * x - 4) + 2, 0.f), 0.99f);};

        col = {red(val), green(val), blue(val)};
    }
};


int main()
{
    float width{720}, height{720};
    kki::Context window(width, height, "Name");
    window.bind();

    auto shader = std::make_shared<kki::Shader>("../src/shaders/pixies.glsl");
    auto background_shader = std::make_shared<kki::Shader>("../src/shaders/texture.glsl");
    background_shader->use();
    background_shader->setFloat("colour", 1, 0, 0);

    size_t number_of_pixies = 100000;

    NoiseBackground background(500, 500);
    PlanktonPixies pixies(background, number_of_pixies, true);


    std::chrono::milliseconds frame_len(30);
    while (!window.should_close())
    {
        auto begin = std::chrono::steady_clock::now();
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        background.update();
        //background.draw(background_shader);

        pixies.updateAll();
        pixies.draw(shader);

        window.swap_buffers();
        auto end = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(frame_len - (end - begin));
        window.reset_viewport();
        window.poll_events();
    }
    return 0;
}


