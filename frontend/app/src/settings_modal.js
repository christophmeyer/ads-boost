import Button from "react-bootstrap/Button";
import Modal from "react-bootstrap/Modal";
import Form from "react-bootstrap/Form";
import InputGroup from "react-bootstrap/InputGroup";
import { useState } from "react";

function SettingsModal({
  timeout,
  handleTimeoutChange,
  onHide,
  show,
  ...modalProps
}) {
  const [local_timeout, setTimeout] = useState(timeout);

  const saveSettings = () => {
    handleTimeoutChange(local_timeout);
  };

  return (
    <Modal
      {...modalProps}
      className="settings-modal"
      size="lg"
      aria-labelledby="settings-modal"
      centered
      show={show}
      onHide={onHide}
    >
      <Modal.Header>
        <Modal.Title>Settings</Modal.Title>
      </Modal.Header>
      <Modal.Body>
        <InputGroup className="mb-3">
          <Form.Control
            placeholder={timeout}
            className="timeout-input-form"
            onChange={(e) => setTimeout(e.target.value)}
          />
          <InputGroup.Text id="timeout-setting">
            timeout in seconds
          </InputGroup.Text>
        </InputGroup>
      </Modal.Body>
      <Modal.Footer>
        <Button onClick={saveSettings}>Save</Button>
      </Modal.Footer>
    </Modal>
  );
}

export default SettingsModal;
