TypeManager = {connectors={}}


function TypeManager.buildWidget()

	TypeManager.widget = guiManager:createWidget()
	TypeManager.widget:loadMainSheet("TypeManager.layout", "TypeManager/")
	
	TypeManager.typeTree = tolua.cast(TypeManager.widget:getWindow("TypeList"), "CEGUI::Tree")
	TypeManager.typeTree:subscribeEvent("ItemSelectionChanged", "TypeManager.TypeList_SelectionChanged")
	
	TypeManager.typeInfoText = CEGUI.toMultiLineEditbox(TypeManager.widget:getWindow("TypeInfoText"), "CEGUI::Tree")

	TypeManager.typeAdapter = EmberOgre.Gui.Adapters.Eris.TypeTreeAdapter:new_local(emberServices:getServerService():getConnection():getTypeService(), TypeManager.typeTree)
	TypeManager.typeAdapter:initialize("root")
	
	TypeManager.widget:registerConsoleVisibilityToggleCommand("typeManager")
	TypeManager.widget:enableCloseButton()
	TypeManager.widget:hide()

end

function TypeManager.CreatedAvatarEntity()
	TypeManager.buildWidget()
end

function TypeManager.TypeList_SelectionChanged(args)

	local typeInfo = TypeManager.typeAdapter:getSelectedTypeInfo()
	
	if typeInfo ~= nil then
	
		local rawTypeData = emberOgre:getEntityFactory():getAuthoringManager():getRawTypeInfoRepository():getRawTypeData(typeInfo:getName())
	
		if rawTypeData:isValid() then
		
			local outstream = std.stringstream:new_local()
			local decoder = Atlas.Message.QueuedDecoder:new_local()
		
			local codec = Atlas.Codecs.XML:new_local(outstream, tolua.cast(decoder, "Atlas::Bridge"))
			local formatter = Atlas.Formatter:new_local(outstream, tolua.cast(codec, "Atlas::Bridge"))
			local encoder = Atlas.Message.Encoder:new_local(tolua.cast(formatter, "Atlas::Bridge"))
			local message = Atlas.Message.MapType:new_local()
			rawTypeData:get():addToMessage(message)
			formatter:streamBegin();
			encoder:streamMessageElement(message);
		
			formatter:streamEnd();
		
			TypeManager.typeInfoText:setText(outstream:str())
		end
	end
	return true
end

connect(TypeManager.connectors, emberOgre.EventCreatedAvatarEntity, TypeManager.CreatedAvatarEntity)
