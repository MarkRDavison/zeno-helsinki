#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <iostream>
#include <format>
#define _CRT_SECURE_NO_WARNINGS
#include <stb_image.h>
#define __STDC_LIB_EXT1__
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <algorithm>
#include <filesystem>
#include <fstream>

constexpr const int AtlasWidth = 4096;
constexpr const int AtlasHeight = 4096;
constexpr const int SDF_GenSize = 64;
constexpr const int Raster_GenSize = 24;

namespace hl
{

    

	FontResource::FontResource(
		const std::string& id,
		ResourceContext& context,
        FontType fontType
	) :
		Resource(id),
		_device(*context.device),
		_commandPool(*context.pool),
        _resourceManager(*context.resourceManager),
		_rootPath(context.rootPath),
        _fontType(fontType)
	{

	}

    bool FontResource::Load()
    {
        auto fontTexturePath = std::format("{}/data/textures/{}.png", _rootPath, GetId());
        auto fontPath = std::format("{}/data/fonts/{}.ttf", _rootPath, GetId());

        if (FT_Init_FreeType(&_ft))
        {
            std::cerr << "Failed to init freetype" << std::endl;
            return false;
        }

        if (FT_New_Face(_ft, fontPath.c_str(), 0, &_face))
        {
            std::cerr << "Failed to create font face" << std::endl;
            return false;
        }

        FT_CharMap found{ 0 };
        FT_CharMap charmap;

        for (auto n{ 0 }; n < _face->num_charmaps; n++)
        {
            charmap = _face->charmaps[n];
            if (charmap)
            {
                found = charmap;
                break;
            }
        }

        if (!found)
        {
            std::cerr << "Failed to find character map" << std::endl;
            return false;
        }

        if (FT_Set_Charmap(_face, found))
        {
            std::cerr << "Failed to set character map" << std::endl;
            return false;
        }

        glm::ivec2 penPosition{};

        int maxY = 0;

        auto size = _fontType == FontType::Rasterised
            ? Raster_GenSize
            : SDF_GenSize;
        if (FT_Set_Pixel_Sizes(_face, 0, size))
        {
            std::cerr << "Failed to set pixel sizes" << std::endl;
            return false;
        }

        FT_ULong c;
        FT_UInt glyph_index;
        c = FT_Get_First_Char(_face, &glyph_index);

        if (std::filesystem::exists(fontTexturePath) && loadFontConfigFile())
        {
            return Resource::Load();
        }

        std::vector<uint8_t> pixels;
        pixels.resize(4 * AtlasWidth * AtlasHeight);
        
        while (glyph_index != 0)
        {
            bool renderable{ true };

            glyph_index = FT_Get_Char_Index(_face, c);

            if (FT_Load_Glyph(_face, glyph_index, FT_LOAD_RENDER))
            {
                std::cerr << "Failed to load Glyph" << std::endl;
                renderable = false;
            }

            auto renderMode = _fontType == FontType::Rasterised 
                ? FT_RENDER_MODE_NORMAL 
                : FT_RENDER_MODE_SDF;

            if (FT_Render_Glyph(_face->glyph, renderMode))
            {
                std::cerr << "Failed to render Glyph" << std::endl;
                renderable = false;
            }

            if (_face->glyph->bitmap.buffer == NULL || 
                _face->glyph->bitmap.width == 0 || 
                _face->glyph->bitmap.rows == 0)
            {
                renderable = false;
            }

            //some glyphs are not to be rendered, nothing to draw for a space for     example
            if (renderable)
            {
                std::vector<unsigned char> r_buffer{};

                unsigned int glyph_width{ 0 };
                unsigned int glyph_height{ 0 };
                int glyph_pitch{ 0 };

                glyph_width = _face->glyph->bitmap.width;
                glyph_height = _face->glyph->bitmap.rows;
                glyph_pitch = _face->glyph->bitmap.pitch;

                r_buffer.resize(glyph_width * glyph_height, 0);

                auto buffer = _face->glyph->bitmap.buffer;
                int index{ 0 };

                //reading the bitmap buffer, saving data as alpha 
                for (unsigned int y{ 0 }; y < glyph_height; y++)
                {
                    const uint8_t* row_buffer = buffer + y * glyph_pitch;
                    for (unsigned int x{ 0 }; x < glyph_width; x++)
                    {
                        auto gray_value = row_buffer[x];
                        r_buffer[index++] = gray_value;
                    }
                }

                auto character = FontCharacter
                {
                  .glyph_index = glyph_index,
                  .size = glm::ivec2(glyph_width, glyph_height),
                  .start = glm::ivec2(_face->glyph->bitmap_left, _face->glyph->bitmap_top),
                  .bearing = glm::ivec2(_face->glyph->metrics.horiBearingX, _face->glyph->metrics.horiBearingY),
                  .advanceX = _face->glyph->advance.x,
                  .glyph_area = glyph_width * glyph_height,
                  .tex_x_offset = penPosition.x,
                  .tex_y_offset = penPosition.y
                };

                for (unsigned int glyphY = 0; glyphY < glyph_height; ++glyphY)
                {
                    for (unsigned int glyphX = 0; glyphX < glyph_width; ++glyphX)
                    {
                        unsigned int targetAtlasX = penPosition.x + glyphX;
                        unsigned int targetAtlasY = penPosition.y + glyphY;

                        pixels[4 * (targetAtlasY * AtlasWidth + targetAtlasX) + 0] = r_buffer[glyphY * glyph_width + glyphX];
                        pixels[4 * (targetAtlasY * AtlasWidth + targetAtlasX) + 1] = 0;
                        pixels[4 * (targetAtlasY * AtlasWidth + targetAtlasX) + 2] = 0;
                        pixels[4 * (targetAtlasY * AtlasWidth + targetAtlasX) + 3] = 255;
                    }
                }

                penPosition.x += character.size.x + 1;

                _characters[glyph_index] = character;

                if (maxY < penPosition.y + character.size.y + 1)
                {
                    maxY = penPosition.y + character.size.y + 1;
                }

                if (penPosition.x + character.size.x + 1 >= AtlasWidth)
                {
                    penPosition = glm::ivec2(0, maxY);
                }

                if (penPosition.y + character.size.y + 1 >= AtlasHeight)
                {
                    throw std::runtime_error("ATLAS TOO SMALL");
                }
            }

            c = FT_Get_Next_Char(_face, c, &glyph_index);
        }


        if (!stbi_write_png(fontTexturePath.c_str(), AtlasWidth, AtlasHeight, 4, pixels.data(), AtlasWidth * 4))
        {
            FT_Done_Face(_face);
            FT_Done_FreeType(_ft);
            return false;
        }

        writeFontConfigFile();

		return Resource::Load();
	}

    std::vector<Vertex22D> FontResource::generateTextVertexes(const std::string& text, unsigned size) const
    {
        std::vector<Vertex22D> vert;

        float x{ 0.0f };
        float y{ 0.0f };

        float scale = 1.0f;

        if (_fontType == FontType::Rasterised)
        {
            scale = (float)size / (float)Raster_GenSize;
        }
        else if (_fontType == FontType::SignedDistanceField)
        {
            scale = (float)size / (float)SDF_GenSize;
        }
        else
        {
            throw std::runtime_error("INVALID FONT TYPE");
        }

        for (const auto& c : text)
        {
            auto const glyph_index = FT_Get_Char_Index(_face, c);

            if (_characters.count(glyph_index) == 0)
            {
                // Some random space
                x += 5.0f;
                continue;
            }

            const auto& ch = _characters.at(glyph_index);

            //non renderable glyphs, remember ? right now I'm treating it as a 5 pixels   space.
            if (ch.size.x == 0 && ch.size.y == 0)
            {
                x += 5;
                continue;
            }

            //remember when I told you to read the documentation ? Did you do it ?
            //if you want to understand more about glyph metrics
            //go here https://freetype.org/freetype2/docs/glyphs/glyphs-3.html

              //here we are computing the x and y position, to simplify and if I understood  
              //well, bearing is the metrics telling you what space you need to correctly  
              //place the glyph
              //the scale allows me to change the size of the text
            float xpos = x + ch.bearing.x / 64.0f * scale;
            float ypos = y - ch.bearing.y / 64.0f * scale;

            //compute the size of the glyph
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            //you will need the total size of your atlas texture
            float const width{ static_cast<float>(AtlasWidth) };
            float const height{ static_cast<float>(AtlasHeight) };

            //here we are computing the UV coordinates (textures coordinates to fetch the      
            //data into the texture in the fragment shader
            float u0{ ch.tex_x_offset / width };
            float v0{ (ch.tex_y_offset + ch.size.y) / height };
            float u1{ (ch.tex_x_offset + ch.size.x) / width };
            float v1{ ch.tex_y_offset / height };

            //actually creating the six vertices
            Vertex22D vertex_1{
              { xpos, ypos + h},
              { u0, v0 } };

            Vertex22D vertex_2{
              { xpos, ypos},
              { u0, v1 } };

            Vertex22D vertex_3{
              { xpos + w, ypos},
              { u1, v1 } };

            Vertex22D vertex_4{
              { xpos, ypos + h},
              { u0, v0 } };

            Vertex22D vertex_5{
              { xpos + w, ypos},
              { u1, v1 } };

            Vertex22D vertex_6{
              { xpos + w, ypos + h},
              { u1, v0 } };

            vert.emplace_back(vertex_1);
            vert.emplace_back(vertex_2);
            vert.emplace_back(vertex_3);
            vert.emplace_back(vertex_4);
            vert.emplace_back(vertex_5);
            vert.emplace_back(vertex_6);

            //advance is the metrics that gives you the space needed for each glyph to     
            //not overlap with the next glyph
            x += (ch.advanceX >> 6) * scale;
        }

        return vert;
    }

	void FontResource::Unload()
	{
		if (IsLoaded())
		{
            FT_Done_Face(_face);
            FT_Done_FreeType(_ft);
			Resource::Unload();
		}
	}

    void FontResource::writeFontConfigFile() const
    {
        auto fontDataPath = std::format("{}/data/fonts/{}.config", _rootPath, GetId());


        std::ofstream file;

        file.open(fontDataPath, std::ios::out | std::ios::trunc);
        if (file.good())
        {
            for (const auto& [glyphIndex, character] : _characters)
            {
                file << "GI " << glyphIndex << '\n';
                file << "si " << character.size.x << " " << character.size.y << '\n';
                file << "st " << character.start.x << " " << character.start.y << '\n';
                file << "be " << character.bearing.x << " " << character.bearing.y << '\n';
                file << "ad " << character.advanceX << '\n';
                file << "ga " << character.glyph_area << '\n';
                file << "tx " << character.tex_x_offset << " " << character.tex_y_offset << '\n';
                file << "uv " << character.texUv.x << " " << character.texUv.y << '\n' << '\n';
            }

            file.close();
        }
    }

    bool FontResource::loadFontConfigFile()
    {
        auto fontDataPath = std::format("{}/data/fonts/{}.config", _rootPath, GetId());

        _characters.clear();

        std::ifstream file(fontDataPath);
        if (!file.is_open())
        {
            return false;
        }

        FontCharacter* current = nullptr;

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string key;
            iss >> key;

            if (key.starts_with('#') ||
                line.empty())
            {
                continue;
            }
            else if (key.starts_with("GI"))
            {
                unsigned int glyphIndex; iss >> glyphIndex;
                assert(_characters.contains(glyphIndex) == 0);
                _characters.insert({ glyphIndex, FontCharacter{} });
                current = &_characters.at(glyphIndex);
                current->glyph_index = glyphIndex;
            }
            else if (key.starts_with("si"))
            {
                assert(current != nullptr);
                iss >> current->size.x >> current->size.y;
            }
            else if (key.starts_with("st"))
            {
                assert(current != nullptr);
                iss >> current->start.x >> current->start.y;
            }
            else if (key.starts_with("be"))
            {
                assert(current != nullptr);
                iss >> current->bearing.x >> current->bearing.y;
            }
            else if (key.starts_with("ad"))
            {
                assert(current != nullptr);
                iss >> current->advanceX;
            }
            else if (key.starts_with("ga"))
            {
                assert(current != nullptr);
                iss >> current->glyph_area;

            }
            else if (key.starts_with("tx"))
            {
                assert(current != nullptr);
                iss >> current->tex_x_offset >> current->tex_y_offset;

            }
            else if (key.starts_with("uv"))
            {
                assert(current != nullptr);
                iss >> current->texUv.x >> current->texUv.y;
            }
            else
            {
                throw std::runtime_error("Unhandled font config key.");
            }
        }

        file.close();

        return true;
    }

}