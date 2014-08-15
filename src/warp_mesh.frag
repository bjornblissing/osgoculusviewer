#version 110
    
uniform sampler2D Texture;
    
varying vec4 oColor;
varying vec2 oTexCoord0;
varying vec2 oTexCoord1;
varying vec2 oTexCoord2;
    
void main()
{
   gl_FragColor.r = oColor.r * texture2D(Texture, oTexCoord0).r;
   gl_FragColor.g = oColor.g * texture2D(Texture, oTexCoord1).g;
   gl_FragColor.b = oColor.b * texture2D(Texture, oTexCoord2).b;
   gl_FragColor.a = 1.0;
}
