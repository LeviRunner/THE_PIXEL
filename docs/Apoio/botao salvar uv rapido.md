(
    max modify mode
    local objs = selection as array
    local pasta = maxFilePath
    if pasta ==  do pasta = @EThe_Pixelartsmodelos_3D0_Personagens1_FEMALE
    local sufixo = if (findString (toUpper pasta) FEMALE != undefined) then _F else _M
    
    for obj in objs where superclassof obj == GeometryClass do (
        select obj
        local uvw = obj.modifiers[#Unwrap_UVW]
        if uvw != undefined do (
            modPanel.setCurrentObject uvw
            uvw.renderuv_width = 1024
            uvw.renderuv_height = 1024
            uvw.renderuv_fillmode = 1
            uvw.renderuv_fillcolor = color 255 255 255
            uvw.renderuv_seamcolor = color 0 255 0
            uvw.renderuv_edgecolor = color 150 150 150
            uvw.renderuv_showoverlap = false
            
            local nomeLimpo = obj.name
            if (findString (toUpper nomeLimpo) _F == undefined) and (findString (toUpper nomeLimpo) _M == undefined) do (
                nomeLimpo = nomeLimpo + sufixo
            )
            local caminhoFinal = pasta + nomeLimpo + .png
            uvw.renderUV caminhoFinal
            print (Salvo  + caminhoFinal)
        )
    )
    messageBox Todos os PNGs foram renderizados e salvos na pasta!
)