#version 410

// Input
layout(location = 0) in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;
uniform int outputMode = 2; // 0: original, 1: grayscale, 2: blur
uniform bool filterType;

uniform float brightnessValue;
uniform float contrastFactor;

uniform bool shadeTint;
uniform float tint;
uniform float shade;

// Output
layout(location = 0) out vec4 out_color;

// Local variables
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y); // Flip texture

const int arrayDim = 25;

const mat3 gaussianKernel3x3 = mat3(
        1.0, 2.0, 1.0,
        2.0, 4.0, 2.0,
        1.0, 2.0, 1.0);


const mat3 prewittKernelX = mat3(
        -1.0, 0.0, 1.0,
		-1.0, 0.0, 1.0,
		-1.0, 0.0, 1.0);


const mat3 prewittKernelY = mat3(
        -1.0, -1.0, -1.0,
		 0.0,  0.0,  0.0,
		 1.0,  1.0,  1.0);


const mat3 sobelKernelX = mat3(
        -1.0, 0.0, 1.0,
		-2.0, 0.0, 2.0,
		-1.0, 0.0, 1.0);


const mat3 sobelKernelY = mat3(
		-1.0, -2.0, -1.0,
		 0.0,  0.0,  0.0,
		 1.0,  2.0,  1.0);


vec4 grayscale()
{
    vec4 color = texture(textureImage, textureCoord);
    float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;

    return vec4(gray, gray, gray,  0);
}


vec4 negative()
{
    vec4 color = texture(textureImage, textureCoord);
    float R = 1 - color.r;
    float G = 1 - color.g;
    float B = 1 - color.b;

    return vec4(R, G, B,  0);
}


vec4 brightness()
{
	vec4 color = texture(textureImage, textureCoord);

    return vec4((color.rgb + vec3(brightnessValue)), 1.0);
}


vec4 contrast()
{
	vec4 color = texture(textureImage, textureCoord);

    return vec4((color.rgb - vec3(0.5)) * contrastFactor + vec3(0.5), 1.0);
}


vec4 shade_tint()
{
    vec4 color = texture(textureImage, textureCoord);
    float R = color.r;
    float G = (shadeTint == false) ? (color.g * (1 - shade)) : (color.g + (1 - color.g) * tint);
    float B = color.b;

    return vec4(R, G, B,  0);
}


float sort(float array[arrayDim])
{
    for (int i = 0; i < arrayDim - 1; i++)
    {
        for (int j = i + 1; j < arrayDim; j++)
        {
            if(array[i] >= array[j])
            {
                float aux = array[i];
                array[i] = array[j];
                array[j] = aux;
            }
        }
    }
    int index = (0 + arrayDim) / 2 + 1;
    
    return array[index];
}


vec4 median()
{
    vec2 texelSize = 1.0 / screenSize;

    float array[arrayDim];
    for (int i = - 2; i <= 2; i++)
    {
        for (int j = -2; j <= 2; j++)
        {
            vec4 color = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
            float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
            array[(i + 2) * 5 + (j + 2)] = gray;
        }
    }
    float val = sort(array);
    
    return vec4(val, val, val, 1.0);
}


vec4 blur(int blurRadius)
{
    vec2 texelSize = 1.0 / screenSize;

    vec4 sum = vec4(0);
    for (int i = -blurRadius; i <= blurRadius; i++)
    {
        for (int j = -blurRadius; j <= blurRadius; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }
    }
    float samples = pow((2 * blurRadius + 1), 2);

    return sum / samples;
}


vec4 gaussian()
{
    vec2 texelSize = 1.0 / screenSize;

    float neighbors[9];
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            vec4 color = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
            float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
            neighbors[(i + 1) * 3 + (j + 1)] = gray;
        }
    }

    vec4 sum = vec4(0);
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            sum += neighbors[(i + 1) * 3 + (j + 1)] * gaussianKernel3x3[i + 1][j + 1];
        }
    }
    float samples = 16;

    return sum / samples;
}


vec4 sharpen()
{
    vec2 texelSize = 1.0 / screenSize;

    vec4 sum = vec4(0.0);
    sum += -1.0 * texture2D(textureImage, textureCoord + vec2(-1.0,  0.0) * texelSize);
    sum += -1.0 * texture2D(textureImage, textureCoord + vec2( 0.0, -1.0) * texelSize);
    sum +=  5.0 * texture2D(textureImage, textureCoord + vec2( 0.0,  0.0) * texelSize);
    sum += -1.0 * texture2D(textureImage, textureCoord + vec2( 0.0,  1.0) * texelSize);
    sum += -1.0 * texture2D(textureImage, textureCoord + vec2( 1.0,  0.0) * texelSize);
    
    return sum;
}


vec4 prewitt()
{
    vec2 texelSize = 1.0 / screenSize;

    float neighbors[9];
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            vec4 color = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
            float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
            neighbors[(i + 1) * 3 + (j + 1)] = gray;
        }
    }

    float Gx = 0;
    float Gy = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            Gx += neighbors[(i + 1) * 3 + (j + 1)] * prewittKernelX[i + 1][j + 1];
            Gy += neighbors[(i + 1) * 3 + (j + 1)] * prewittKernelY[i + 1][j + 1];
        }
    }
    float G = sqrt(Gx * Gx + Gy * Gy);
    
    return vec4(G, G, G,  0);
}


float convolve(mat3 kernel, mat3 image)
{
    float result = 0.0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result += kernel[i][j] * image[i][j];
        }
    }

    return result;
}


float convolveComponent(mat3 kernelX, mat3 kernelY, mat3 image)
{
    vec2 result;
    result.x = convolve(kernelX, image);
    result.y = convolve(kernelY, image);

    return clamp(length(result), 0.0, 1.0);
}


vec4 sobel()
{
    vec2 texelSize = 1.0 / screenSize;

    vec4 colors[9];
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            colors[(i + 1) * 3 + (j + 1)] = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }
    }

    mat3 imageR, imageG, imageB;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            imageR[i][j] = colors[i * 3 + j].r;
            imageG[i][j] = colors[i * 3 + j].g;
            imageB[i][j] = colors[i * 3 + j].b;
        }
    }
    
    vec4 color;
    color.r = convolveComponent(sobelKernelX, sobelKernelY, imageR);
    color.g = convolveComponent(sobelKernelX, sobelKernelY, imageG);
    color.b = convolveComponent(sobelKernelX, sobelKernelY, imageB);

    return color;
}


vec4 canny()
{
    vec2 texelSize = 1.0 / screenSize;

    float neighbors[9];
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            vec4 color = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
            float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
            neighbors[(i + 1) * 3 + (j + 1)] = gray;
        }
    }

    vec4 sum = vec4(0);
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            sum += neighbors[(i + 1) * 3 + (j + 1)] * gaussianKernel3x3[i + 1][j + 1];
        }
    }

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            vec4 color = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
            float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
            neighbors[(i + 1) * 3 + (j + 1)] = gray;
        }
    }
    float Gx = 0;
    float Gy = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            Gx += neighbors[(i + 1) * 3 + (j + 1)] * sobelKernelX[i + 1][j + 1];
            Gy += neighbors[(i + 1) * 3 + (j + 1)] * sobelKernelY[i + 1][j + 1];
        }
    }
    float G = sqrt(Gx * Gx + Gy * Gy);
    float dir = atan(Gy, Gx);

    vec4 pixel = texture(textureImage, textureCoord);
    float q = 1;
    float r = 1;

    if ((0.0 <= dir && dir < 22.5) || (157.5 <= dir && dir <= 180.0))
    {
        q = G;
    }

    return vec4(G, G, G,  0);
}


void main()
{
    switch (outputMode)
    {
		case 1:
        {
			out_color = (filterType == false) ? grayscale() : median();
            break;
        }

        case 2:
        {
            out_color = (filterType == false) ? negative() : blur(3);
            break;
        }
        case 3:
        {
            out_color = (filterType == false) ? brightness() : sharpen();
            break;
        }
        case 4:
        {
            out_color = (filterType == false) ? contrast() : prewitt();
            break;
        }
        case 5:
        {
            out_color = (filterType == false) ? shade_tint() : sobel();
            break;
        }

        default:
            out_color = texture(textureImage, textureCoord);
            break;
    }
}
