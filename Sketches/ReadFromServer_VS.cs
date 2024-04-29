using System;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

class Program
{
    static void Main(string[] args)
    {
        while (true)
        {
            string brokerAddress = "cheatreciever.cloud.shiftr.io";
            string clientId = "GOIDA"; // any id (unique for server)
            string username = "cheatreciever"; // Shiftr.io login
            string password = "Slsk3E6qcEI0mM84"; // Shiftr.io password
            string topic = "Danger-data"; // topic to read from

            MqttClient client = new MqttClient(brokerAddress);
            client.Connect(clientId, username, password);

            client.MqttMsgPublishReceived += Client_MqttMsgPublishReceived;

            client.Subscribe(new string[] { topic }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE });

            Console.ReadLine();

            // Задержка на 3 секунды
            Thread.Sleep(3000);
        }

        //client.Disconnect();
    }

    private static void Client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
    {
        string message = System.Text.Encoding.UTF8.GetString(e.Message);
        Console.WriteLine($"Данные с сервера получены: {message}");
    }
}
