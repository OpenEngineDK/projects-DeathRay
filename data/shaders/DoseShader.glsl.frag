uniform sampler3D intensityTex;
uniform sampler3D doseTex;

void main()
{
    vec4 intensity = texture3D(intensityTex, gl_TexCoord[0].xyz);
    vec4 dose = texture3D(doseTex, gl_TexCoord[0].xyz);

    gl_FragColor = intensity + dose;
}
