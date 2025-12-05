class CfgPatches
{
    class Pay_For_Fuel
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};

class CfgMods
{
    class Pay_For_Fuel
    {
        type = "mod";
        dir = "Pay_For_Fuel";
        name = "Pay For Fuel";
        credits = "";
        author = "Pay For Fuel";
        authorID = "0";
        version = "1.0";
        dependencies[] = {"World"};

        class defs
        {
            class worldScriptModule
            {
                value = "";
                files[] = {"Pay_For_Fuel/scripts/4_world"};
            };
        };
    };
};
