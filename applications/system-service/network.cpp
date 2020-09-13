#include <QSet>

#include "network.h"
#include "wifiapi.h"

//#define DEBUG

QSet<QString> none{
    "NONE",
    "WPA-NONE"
};
QSet<QString> psk{
    "WPA-PSK",
    "FT-PSK",
    "WPA-PSK-SHA256"
};
QSet<QString> eap{
    "WPA-EAP",
    "FT-EAP",
    "FT-EAP-SHA384",
    "WPA-EAP-SHA256",
    "WPA-EAP-SUITE-B",
    "WPA-EAP-SUITE-B-192"
};
QSet<QString> sae{
    "SAE"
};

Network::Network(QString path, QString ssid, QVariantMap properties, QObject* parent)
: QObject(parent), m_path(path), networks(), m_properties(properties), m_ssid(ssid),
  m_enabled(false){
    auto proto = properties["key_mgmt"].toString();
    if(psk.contains(proto)){
        m_protocol = "psk";
    }else if(eap.contains(proto)){
        m_protocol = "eap";
    }else if(none.contains(proto)){
        m_protocol = "open";
    }else if(sae.contains(proto)){
        m_protocol = "sae";
    }else{
        m_protocol = "psk";
    }
    QDBusConnection::systemBus().registerObject(path, this);
}
void Network::registerNetwork(){
    auto api = (WifiAPI*)parent();
    for(auto interface : api->getInterfaces()){
        bool found = false;
        for(auto network : networks){
            if(interface->path() == ((Interface*)network->parent())->path()){
                found = true;
                break;
            }
        }
        if(!found){
#ifdef DEBUG
            qDebug() << realProps();
#endif
            QDBusObjectPath path = interface->AddNetwork(realProps());
            auto network = new INetwork(WPA_SUPPLICANT_SERVICE, path.path(), QDBusConnection::systemBus(), interface);
            networks.append(network);
            network->setEnabled(m_enabled);
        }
    }
}
