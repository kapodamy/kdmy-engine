--[[ Generated with https://github.com/TypeScriptToLua/TypeScriptToLua ]]
function tprint (tbl, indent)
    if not indent then indent = 0 end
    for k, v in pairs(tbl) do
      formatting = string.rep("  ", indent) .. k .. ": "
      if type(v) == "table" then
        print(formatting)
        tprint(v, indent+1)
      elseif type(v) == 'boolean' then
        print(formatting .. tostring(v))		
      else
        print(formatting .. v)
      end
    end
  end
function dump_table(placeholder)
    if placeholder == nil then
        print("<null>")
        return
    end
    tprint(placeholder, 0)
end

stage = week_get_stage_layout()
placeholder1 = stage:get_placeholder("test1")
print("test1\n")
dump_table(placeholder1)
placeholder2 = stage:get_placeholder("test2")
print("test2\n")
dump_table(placeholder2)
placeholder3 = stage:get_placeholder("test3")
print("test3\n")
dump_table(placeholder3)
