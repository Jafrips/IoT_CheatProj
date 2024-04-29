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
            string clientId = "GOIDA"; // Вам нужно использовать свой клиентский идентификатор
            string username = "cheatreciever"; // Ваш логин от Shiftr.io
            string password = "Slsk3E6qcEI0mM84"; // Ваш пароль от Shiftr.io
            string topic = "Danger-data"; // Топик, откуда вы хотите читать данные

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
