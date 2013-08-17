#Simple OpenGL Image Library *Ext*#
[The original SOIL from July 7, 2008](http://www.lonesock.net/soil.html) is released with **Public Domain Licence**.

The **Ext** version contains the following changes:

* **[in progress]** Added option to use immutable texture storage
 * When possible (when extension is supported) library can create texture using glTextureStorage and then glTexSubImage
 * See more here [OpenGL wiki](http://www.opengl.org/wiki/Texture_Storage)
* **[in progress]** Mipmap generation using glGenerateMipmap
 * Original library scaled image to be POT and then used custom procedure to generate mipmaps. This can take some time.
 * This change use glGenerateMipmapEXT (if this extension is available)
* **[done]** upgraded to the version **1.33** of [stb_image](http://www.nothings.org/stb_image.c)
 * stb_image_write.c/.h were created. Those files contain missing functionality that was cut from the newer version of stbi (cut in 1.22)  
* **[done]** Using only Modern OpenGL functionalities (when possible)
 * *CLAMP\_TO\_EDGE* instead of *GL\_CLAMP*
 *  *glGetStringi* instead of *glGetString(GL\_EXTENSIONS)* 
* **[done]** Visual Studio 2012 Express solution
 * It compiles without any warnings: used *\_CRT_SECURE_NO_WARNINGS* and several code changes regarding *int/float* cast and *signed/unsigned* comparisons.

###Notes###
* Original functionality is preserved and should work as expected.
* Doc comments were changed to describe changes