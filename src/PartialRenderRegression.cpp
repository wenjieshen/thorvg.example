/*
 * Copyright (c) 2026 ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Example.h"


struct UserExample : tvgexam::Example
{
    using tvgexam::Example::Example;

    tvg::Shape* background = nullptr;
    tvg::Shape* outerClipper = nullptr;
    bool moved = false;

    void label(tvg::Canvas* canvas, const char* value, float x, float y, float size)
    {
        auto text = tvg::Text::gen();
        text->font("PublicSans-Regular");
        text->size(size);
        text->text(value);
        text->fill(40, 40, 40);
        text->translate(x, y);
        canvas->add(text);
    }

    bool content(tvg::Canvas* canvas, const tvg::toolkit::App::Size& size) override
    {
        background = tvg::Shape::gen();
        background->appendRect(0, 0, size.w, size.h);
        background->fill(255, 255, 255);
        canvas->add(background);

        if (!tvgexam::verify(tvg::Text::load(EXAMPLE_DIR"/font/PublicSans-Regular.ttf"))) return false;

        label(canvas, "Nested clip regression", 24, 20, 24);
        label(canvas, "PR #4878 / 516f60e | Partial rendering: draw(false)", 24, 58, 14);
        label(canvas, "Space or click: move / reset. Esc closes.", 24, 82, 13);

        auto panel = tvg::Shape::gen();
        panel->appendRect(24, 120, 432, 348);
        panel->fill(255, 255, 255, 0);
        panel->strokeFill(190, 190, 190);
        panel->strokeWidth(1);
        canvas->add(panel);

        label(canvas, "Nested clips", 40, 134, 19);
        label(canvas, "Outer: 80 x 180; inner: 150 x 150, r=20", 40, 168, 13);
        label(canvas, "Only outer clip moves: (0,0) -> (50,0)", 40, 188, 13);
        label(canvas, "After move: red only at local x=50..130.", 40, 428, 13);

        // Place the 200 x 200 repro area without scaling its geometry.
        // Clip geometry is relative to its parent, so translate this static
        // wrapper rather than the clipped outer scene itself.
        auto clipPanel = tvg::Scene::gen();
        clipPanel->translate(48, 220);
        auto outer = tvg::Scene::gen();
        auto inner = tvg::Scene::gen();

        auto red = tvg::Shape::gen();
        red->appendRect(0, 0, 180, 180);
        red->fill(255, 0, 0);
        inner->add(red);

        auto rounded = tvg::Shape::gen();
        rounded->appendRect(0, 0, 150, 150, 20, 20);
        inner->clip(rounded);
        outer->add(inner);

        outerClipper = tvg::Shape::gen();
        outerClipper->appendRect(0, 0, 80, 180);
        outer->clip(outerClipper);
        clipPanel->add(outer);
        canvas->add(clipPanel);

        return true;
    }

    bool toggle(tvg::Canvas* canvas)
    {
        moved = !moved;
        outerClipper->translate(moved ? 50 : 0, 0);
        // Invalidate the unchanged background only on reset to erase stale
        // pixels. Moving still changes only the outer clipper.
        if (!moved) background->transform(background->transform());
        return tvgexam::verify(canvas->update());
    }

    bool keydown(tvg::Canvas* canvas, tvg::toolkit::Key key) override
    {
        if (key == tvg::toolkit::Key::Space) return toggle(canvas);
        return tvgexam::Example::keydown(canvas, key);
    }

    bool clickdown(tvg::Canvas* canvas, int32_t, int32_t, uint8_t) override
    {
        return toggle(canvas);
    }
};


int main(int argc, char** argv)
{
    auto params = tvgexam::options(argc, argv, {480, 496});
    // Keep the toolkit's default smart-rendering canvas and draw(false).
    return tvgexam::run(new UserExample(params), params);
}
