import React from "react";
import Table from "react-bootstrap/Table";

function bool_value_to_string(value) {
  if (value === "NA") {
    return "";
  } else if (value === "true") {
    return "ON";
  } else if (value === "false") {
    return "OFF";
  }
}
function ContactsTable(props) {
  return (
    <div className="table-container">
      <Table className="contact-table" variant="dark" striped hover size="sm">
        <thead>
          <tr>
            <th>ICAO</th>
            <th>Callsign</th>
            <th>LAT</th>
            <th>LON</th>
            <th>ALT</th>
            <th>Selected ALT</th>
            <th>Speed</th>
            <th>Track</th>
            <th>Selected Heading</th>
            <th>VRate</th>
            <th>Type</th>
            <th>Autopilot</th>
            <th>Approach Mode</th>
            <th>Altitude Hold Mode</th>
            <th>TCAS Mode</th>
            <th>Messages</th>
            <th> Last Seen</th>
          </tr>
        </thead>
        <tbody>
          {props.contact_data &&
            props.contact_data.contacts
              .filter((contact) => contact.last_seen < props.timeout)
              .map((contact) => (
                <tr key={contact.icao}>
                  <td>{contact.icao}</td>
                  <td>{contact.callsign}</td>
                  <td>
                    {contact.position_status === "KNOWN" ? contact.lat : ""}
                  </td>
                  <td>
                    {contact.position_status === "KNOWN" ? contact.lon : ""}
                  </td>
                  <td>
                    {contact.altitude_type !== "UNDETERMINED"
                      ? contact.altitude
                      : ""}
                  </td>
                  <td>
                    {contact.selected_altitude_status !== "UNDETERMINED"
                      ? contact.selected_altitude
                      : ""}
                  </td>
                  <td>
                    {contact.speed_type !== "UNDETERMINED" ? contact.speed : ""}
                  </td>
                  <td>
                    {contact.heading_type !== "UNDETERMINED"
                      ? contact.heading
                      : ""}
                  </td>
                  <td>
                    {contact.selected_heading_status !== "UNDETERMINED"
                      ? contact.selected_heading
                      : ""}
                  </td>
                  <td>
                    {contact.vertical_rate_status !== "UNDETERMINED"
                      ? contact.vertical_rate
                      : ""}
                  </td>
                  <td>{contact.aircraft_category}</td>
                  <td>{bool_value_to_string(contact.autopilot)}</td>
                  <td>{bool_value_to_string(contact.approach_mode)}</td>
                  <td>{bool_value_to_string(contact.altitude_hold_mode)}</td>
                  <td>{bool_value_to_string(contact.tcas_operational)}</td>
                  <td>{contact.n_messages}</td>
                  <td>{contact.last_seen}</td>
                </tr>
              ))}
        </tbody>
      </Table>
    </div>
  );
}

export default ContactsTable;
