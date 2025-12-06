static const string PFF_CONFIG_PATH = "$profile:PayForFuelConfig.json";

class PayForFuelConfigData
{
    int FuelPrice = 25;
    string MessagePrefix = "[Pay For Fuel] ";
    ref array<string> TargetNameFilters;
    ref map<string, int> CurrencyValues;

    void PayForFuelConfigData()
    {
        if (!TargetNameFilters)
        {
            TargetNameFilters = new array<string>;
            TargetNameFilters.Insert("Land_FuelStation");
            TargetNameFilters.Insert("FuelStation");
        }

        if (!CurrencyValues)
        {
            CurrencyValues = new map<string, int>;
            CurrencyValues.Insert("MoneyRuble1", 1);
            CurrencyValues.Insert("MoneyRuble5", 5);
            CurrencyValues.Insert("MoneyRuble10", 10);
            CurrencyValues.Insert("MoneyRuble25", 25);
            CurrencyValues.Insert("MoneyRuble50", 50);
            CurrencyValues.Insert("MoneyRuble100", 100);
        }
    }
}

class PayForFuelConfig
{
    protected static ref PayForFuelConfigData m_Config;

    protected static void Load()
    {
        PayForFuelConfigData config = new PayForFuelConfigData();

        if (FileExist(PFF_CONFIG_PATH))
        {
            JsonFileLoader<PayForFuelConfigData>.JsonLoadFile(PFF_CONFIG_PATH, config);
        }
        else
        {
            JsonFileLoader<PayForFuelConfigData>.JsonSaveFile(PFF_CONFIG_PATH, config);
        }

        EnsureDefaults(config);
        m_Config = config;
    }

    protected static void EnsureDefaults(PayForFuelConfigData config)
    {
        if (!config)
        {
            config = new PayForFuelConfigData();
            return;
        }

        if (!config.TargetNameFilters || config.TargetNameFilters.Count() == 0)
        {
            config.TargetNameFilters = new array<string>;
            config.TargetNameFilters.Insert("Land_FuelStation");
            config.TargetNameFilters.Insert("FuelStation");
        }

        if (!config.CurrencyValues || config.CurrencyValues.Count() == 0)
        {
            config.CurrencyValues = new map<string, int>;
            config.CurrencyValues.Insert("MoneyRuble1", 1);
            config.CurrencyValues.Insert("MoneyRuble5", 5);
            config.CurrencyValues.Insert("MoneyRuble10", 10);
            config.CurrencyValues.Insert("MoneyRuble25", 25);
            config.CurrencyValues.Insert("MoneyRuble50", 50);
            config.CurrencyValues.Insert("MoneyRuble100", 100);
        }
    }

    protected static PayForFuelConfigData Get()
    {
        if (!m_Config)
        {
            Load();
        }

        return m_Config;
    }

    static int GetFuelPrice()
    {
        PayForFuelConfigData config = Get();
        if (config)
        {
            return config.FuelPrice;
        }

        return 0;
    }

    static string GetMessagePrefix()
    {
        PayForFuelConfigData config = Get();
        if (config)
        {
            return config.MessagePrefix;
        }

        return "";
    }

    static ref map<string, int> GetCurrencyValues()
    {
        PayForFuelConfigData config = Get();
        if (config)
        {
            return config.CurrencyValues;
        }

        return null;
    }

    static ref array<string> GetTargetFilters()
    {
        PayForFuelConfigData config = Get();
        if (config)
        {
            return config.TargetNameFilters;
        }

        return null;
    }

    static int GetCurrencyValue(string typeName)
    {
        if (!typeName || typeName == "")
        {
            return 0;
        }

        map<string, int> currencyValues = GetCurrencyValues();
        if (!currencyValues)
        {
            return 0;
        }

        if (currencyValues.Contains(typeName))
        {
            return currencyValues.Get(typeName);
        }

        return 0;
    }

    static bool RequiresPaymentForTarget(Object target)
    {
        if (!target)
        {
            return false;
        }

        string typeName = target.GetType();
        array<string> filters = GetTargetFilters();
        if (!filters)
        {
            return false;
        }
        foreach (string filter : filters)
        {
            if (filter == "")
            {
                continue;
            }

            if (typeName.IndexOf(filter) > -1)
            {
                return true;
            }
        }

        return false;
    }
}

class PayForFuelCurrencyManager
{
    static int GetPlayerCurrency(PlayerBase player)
    {
        if (!player)
        {
            return 0;
        }

        array<ItemBase> currencyItems = new array<ItemBase>;
        GetCurrencyItems(player, currencyItems);

        int total;
        foreach (ItemBase currencyItem : currencyItems)
        {
            total += PayForFuelConfig.GetCurrencyValue(currencyItem.GetType());
        }

        return total;
    }

    static void GetCurrencyItems(PlayerBase player, notnull array<ItemBase> output)
    {
        output.Clear();

        if (!player)
        {
            return;
        }

        GameInventory inventory = player.GetInventory();
        if (!inventory)
        {
            return;
        }

        array<EntityAI> collected = new array<EntityAI>;
        inventory.EnumerateInventory(InventoryTraversalType.PREORDER, collected);

        foreach (EntityAI entity : collected)
        {
            ItemBase item;
            if (!Class.CastTo(item, entity))
            {
                continue;
            }

            if (PayForFuelConfig.GetCurrencyValue(item.GetType()) > 0)
            {
                output.Insert(item);
            }
        }
    }

    static bool DeductCurrency(PlayerBase player, int price)
    {
        if (!player)
        {
            return false;
        }

        if (price <= 0)
        {
            return true;
        }

        array<ItemBase> currencyItems = new array<ItemBase>;
        GetCurrencyItems(player, currencyItems);

        int available;
        foreach (ItemBase currencyItemCheck : currencyItems)
        {
            available += PayForFuelConfig.GetCurrencyValue(currencyItemCheck.GetType());
        }

        if (available < price)
        {
            return false;
        }

        int remaining = price;
        foreach (ItemBase currencyItem : currencyItems)
        {
            if (!currencyItem)
            {
                continue;
            }

            int value = PayForFuelConfig.GetCurrencyValue(currencyItem.GetType());
            if (value <= 0)
            {
                continue;
            }

            currencyItem.Delete();
            remaining -= value;

            if (remaining <= 0)
            {
                return true;
            }
        }

        return false;
    }

    static bool HasEnoughCurrency(PlayerBase player, int price)
    {
        return GetPlayerCurrency(player) >= price;
    }
}

class PayForFuelMessages
{
    static void NotifyInsufficientFunds(PlayerBase player, int price)
    {
        Send(player, "You need " + price.ToString() + " rubles to refuel.");
    }

    static void NotifyPaymentSuccessful(PlayerBase player, int price)
    {
        Send(player, "Paid " + price.ToString() + " rubles for fuel.");
    }

    static void Send(PlayerBase player, string message)
    {
        if (!player || message == "")
        {
            return;
        }

        string formatted = PayForFuelConfig.GetMessagePrefix() + message;
        player.MessageStatus(formatted);
    }
}

modded class ActionRefuel
{
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!super.ActionCondition(player, target, item))
        {
            return false;
        }

        if (!target)
        {
            return true;
        }

        Object targetObject = target.GetObject();
        if (!PayForFuelConfig.RequiresPaymentForTarget(targetObject))
        {
            return true;
        }

        int price = PayForFuelConfig.GetFuelPrice();
        if (PayForFuelCurrencyManager.HasEnoughCurrency(player, price))
        {
            return true;
        }

        PayForFuelMessages.NotifyInsufficientFunds(player, price);
        return false;
    }

    override void OnStartServer(ActionData action_data)
    {
        if (action_data && action_data.m_Player && action_data.m_Target)
        {
            Object targetObject = action_data.m_Target.GetObject();
            if (PayForFuelConfig.RequiresPaymentForTarget(targetObject))
            {
                int price = PayForFuelConfig.GetFuelPrice();
                if (PayForFuelCurrencyManager.DeductCurrency(action_data.m_Player, price))
                {
                    PayForFuelMessages.NotifyPaymentSuccessful(action_data.m_Player, price);
                }
                else
                {
                    PayForFuelMessages.NotifyInsufficientFunds(action_data.m_Player, price);
                    if (action_data.m_Player.GetActionManager())
                    {
                        action_data.m_Player.GetActionManager().Interrupt();
                    }
                    return;
                }
            }
        }

        super.OnStartServer(action_data);
    }
}
