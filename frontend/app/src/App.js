import React from "react";
import ContactsTable from "./contact_table";
import OSMContactsMap from "./osm_contacts_map";
import Accordion from "react-bootstrap/Accordion";
import "leaflet/dist/leaflet.css";
import { useState, useEffect } from "react";
import SettingsModal from "./settings_modal";
import Icon from "@mdi/react";
import { mdiCog } from "@mdi/js";

function App() {
  const [data, setData] = useState(null);
  const [timeout, setTimeout] = useState(10);
  const [initial_position, setInitialPosition] = useState(null);
  const [settingsModalShow, setSettingsModalShow] = React.useState(false);
  const webSocketUrl =
    "ws://" +
    window.location.hostname +
    ":" +
    process.env.REACT_APP_ADSBOOST_PORT +
    process.env.REACT_APP_ADSBOOST_PATH;

  const handleTimeoutChange = (new_timeout) => {
    setTimeout(new_timeout);
  };

  useEffect(() => {
    const successCallback = (position) => {
      setInitialPosition([position.coords.latitude, position.coords.longitude]);
    };

    const errorCallback = (error) => {
      console.log(error);
      // In case of failure set to Berlin
      setInitialPosition([52.52, 13.405]);
    };
    navigator.geolocation.getCurrentPosition(successCallback, errorCallback);
  }, []);

  useEffect(() => {
    const webSocket = new WebSocket(webSocketUrl);

    webSocket.onopen = () => {
      console.log("WebSocket Connected");
    };

    webSocket.onmessage = (event) => {
      try {
        const jsonData = JSON.parse(event.data);
        // Update your state based on jsonData
        setData(jsonData);
      } catch (error) {
        console.error("Error parsing JSON:", error);
        console.error("Got:", event.data);
      }
    };

    webSocket.onerror = (error) => {
      console.error("WebSocket Error:", error);
    };

    // Clean up function
    return () => {
      if (webSocket.readyState === 1) {
        webSocket.close();
        console.log("WebSocket Disconnected");
      }
    };
  }, [webSocketUrl]); // Empty dependency array ensures this runs only on mount

  return (
    <div>
      <div>
        <Accordion defaultActiveKey="0">
          <Accordion.Item eventKey="0">
            <Accordion.Header>Contacts Table</Accordion.Header>
            <Accordion.Body>
              <ContactsTable contact_data={data} timeout={timeout} />
            </Accordion.Body>
          </Accordion.Item>
        </Accordion>
        <OSMContactsMap
          contact_data={data}
          initial_position={initial_position}
          timeout={timeout}
        />
      </div>
      <div>
        <Icon
          path={mdiCog}
          size={2}
          as="input"
          className="settings-sprocket"
          type="button"
          value="Settings"
          onClick={() => setSettingsModalShow(true)}
        />{" "}
      </div>
      <div>
        <SettingsModal
          className="modal-lg"
          show={settingsModalShow}
          timeout={timeout}
          handleTimeoutChange={handleTimeoutChange}
          onHide={() => setSettingsModalShow(false)}
        />
      </div>
    </div>
  );
}

export default App;
