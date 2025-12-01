#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <format>
#define _CRT_SECURE_NO_WARNINGS
#include <msdfgen.h>
#include <msdfgen-ext.h>
#include <stb_image.h>
#define __STDC_LIB_EXT1__
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <algorithm>
#include <filesystem>

namespace hl
{

	FontResource::FontResource(
		const std::string& id,
		ResourceContext& context
	) :
		Resource(id),
		_device(*context.device),
		_commandPool(*context.pool),
        _resourceManager(*context.resourceManager),
		_rootPath(context.rootPath)
	{

	}

	bool FontResource::Load()
	{
        auto fontTexturePath = std::format("{}/data/textures/{}.png", _rootPath, GetId());

        if (std::filesystem::exists(fontTexturePath))
        {
            return Resource::Load();
        }

        auto fontPath = std::format("{}/data/fonts/{}.ttf", _rootPath, GetId());

        const std::size_t Width = 2048;
        const std::size_t Height = 2048;
        const std::size_t Channels = 3;
        std::vector<uint8_t> data;
        data.resize(Width * Height * Channels);

        std::size_t xPos = 0;
        std::size_t yPos = 0;
        std::size_t maxY = 0;

        if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype())
        {
            if (msdfgen::FontHandle* font = loadFont(ft, fontPath.c_str()))
            {
                int i = 0;

                for (char c  = 33; c <= 126 && c >= 33; ++c)
                {
                    i++;
                    std::cout << i << "/" << (127 - 33) << std::endl;
                    msdfgen::Shape shape;
                    if (msdfgen::loadGlyph(shape, font, c, msdfgen::FONT_SCALING_EM_NORMALIZED))
                    {
                        shape.normalize();
                        msdfgen::edgeColoringSimple(shape, 3.0);
                        int width = 64;
                        int height = 64;
                        msdfgen::Bitmap<float, Channels> msdf(width, height);

                        msdfgen::SDFTransformation t(
                            msdfgen::Projection(64.0,
                                msdfgen::Vector2(0.125, 0.125)),
                            msdfgen::Range(0.125));

                        msdfgen::generateMSDF(msdf, shape, t);

                        const float* raw = static_cast<float*>(msdf);

                        if (xPos + width >= Width)
                        {
                            xPos = 0;
                            yPos = maxY;
                            if (yPos >= Height)
                            {
                                throw std::runtime_error("FONT ATLAS TOO SMALL");
                            }
                        }

                        maxY = std::max(maxY, yPos + height);

                        for (int y = 0; y < height; ++y)
                        {
                            int flippedY = height - 1 - y;
                            for (int x = 0; x < width; ++x)
                            {
                                const float* px = raw + (flippedY * width + x) * Channels;
                                uint32_t idx = (uint32_t)((y + yPos) * Width + x + xPos) * Channels;
                                data[idx + 0] = static_cast<uint8_t>(std::clamp(px[0], 0.0f, 1.0f) * 255.0f);
                                data[idx + 1] = static_cast<uint8_t>(std::clamp(px[1], 0.0f, 1.0f) * 255.0f);
                                data[idx + 2] = static_cast<uint8_t>(std::clamp(px[2], 0.0f, 1.0f) * 255.0f);
                            }
                        }

                        xPos += width;
                    }
                }

                if (!stbi_write_png(fontTexturePath.c_str(), Width, Height, Channels, data.data(), Width * Channels))
                {
                    std::cout << "WOOPS" << std::endl;
                }

                msdfgen::destroyFont(font);
            }
            msdfgen::deinitializeFreetype(ft);
        }

		return Resource::Load();
	}

	void FontResource::Unload()
	{
		if (IsLoaded())
		{
			Resource::Unload();
		}
	}

}